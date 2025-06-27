#include "silo/query_engine/actions/aggregated.h"

#include <map>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <arrow/acero/options.h>
#include <arrow/compute/api.h>
#include <arrow/compute/expression.h>
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <nlohmann/json.hpp>

#include "silo/config/database_config.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/table_scan.h"
#include "silo/storage/column_group.h"
#include "silo/storage/table.h"

using silo::query_engine::CopyOnWriteBitmap;

namespace {

std::vector<silo::schema::ColumnIdentifier> parseGroupByFields(
   const silo::schema::TableSchema& schema,
   const std::vector<std::string>& group_by_fields
) {
   std::vector<silo::schema::ColumnIdentifier> group_by_metadata;
   for (const std::string& group_by_field : group_by_fields) {
      auto column = schema.getColumn(group_by_field);
      CHECK_SILO_QUERY(
         column.has_value(), "Metadata field '{}' to group by not found", group_by_field
      );
      CHECK_SILO_QUERY(
         !isSequenceColumn(column.value().type),
         "The Aggregated action does not support sequence-type columns for now."
      );
      group_by_metadata.push_back(column.value());
   }
   return group_by_metadata;
}

const std::string COUNT_FIELD = "count";

}  // namespace

namespace silo::query_engine::actions {

Aggregated::Aggregated(std::vector<std::string> group_by_fields)
    : group_by_fields(std::move(group_by_fields)) {}

void Aggregated::validateOrderByFields(const schema::TableSchema& schema) const {
   const std::vector<silo::schema::ColumnIdentifier> field_identifiers =
      parseGroupByFields(schema, group_by_fields);

   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         field.name == COUNT_FIELD || std::ranges::any_of(
                                         field_identifiers,
                                         [&](const silo::schema::ColumnIdentifier& metadata) {
                                            return metadata.name == field.name;
                                         }
                                      ),
         "The orderByField '{}' cannot be ordered by, as it does not appear in the groupByFields.",
         field.name
      );
   }
}

arrow::Result<QueryPlan> Aggregated::toQueryPlanImpl(
   std::shared_ptr<const storage::Table> table,
   std::shared_ptr<filter::operators::OperatorVector> partition_filter_operators,
   const config::QueryOptions& query_options
) const {
   validateOrderByFields(table->schema);

   if (group_by_fields.empty()) {
      return makeAggregateWithoutGrouping(table, partition_filter_operators, query_options);
   } else {
      return makeAggregateWithGrouping(table, partition_filter_operators, query_options);
   }
}

arrow::Result<QueryPlan> Aggregated::makeAggregateWithoutGrouping(
   std::shared_ptr<const storage::Table> table,
   std::shared_ptr<filter::operators::OperatorVector> partition_filter_operators,
   const config::QueryOptions& /*query_options*/
) const {
   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      [table, partition_filter_operators, produced = false](
      ) mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (produced == true) {
         std::optional<arrow::ExecBatch> result = std::nullopt;
         return arrow::Future{result};
      }
      produced = true;

      int32_t result_count = 0;

      for (const auto& partition_filter_operator : *partition_filter_operators) {
         result_count += partition_filter_operator->evaluate()->cardinality();
      }

      arrow::Int32Builder result_builder{};
      ARROW_RETURN_NOT_OK(result_builder.Append(result_count));

      arrow::Datum datum;
      ARROW_ASSIGN_OR_RAISE(datum, result_builder.Finish());

      ARROW_ASSIGN_OR_RAISE(
         std::optional<arrow::ExecBatch> result, arrow::ExecBatch::Make({datum})
      );
      return arrow::Future{result};
   };

   ARROW_ASSIGN_OR_RAISE(auto arrow_plan, arrow::acero::ExecPlan::Make());

   arrow::acero::SourceNodeOptions options{
      exec_node::columnsToArrowSchema(getOutputSchema(table->schema)),
      std::move(producer),
      arrow::Ordering::Implicit()
   };
   ARROW_ASSIGN_OR_RAISE(
      auto node, arrow::acero::MakeExecNode("source", arrow_plan.get(), {}, options)
   );

   return QueryPlan::makeQueryPlan(arrow_plan, node);
}

arrow::Result<QueryPlan> Aggregated::makeAggregateWithGrouping(
   std::shared_ptr<const storage::Table> table,
   std::shared_ptr<filter::operators::OperatorVector> partition_filter_operators,
   const config::QueryOptions& query_options
) const {
   ARROW_ASSIGN_OR_RAISE(auto arrow_plan, arrow::acero::ExecPlan::Make());

   std::vector<schema::ColumnIdentifier> group_by_fields_identifiers =
      columnNamesToFields(group_by_fields, table->schema);

   arrow::acero::ExecNode* node = arrow_plan->EmplaceNode<exec_node::TableScan>(
      arrow_plan.get(),
      group_by_fields_identifiers,
      partition_filter_operators,
      table,
      query_options.materialization_cutoff
   );

   auto count_options =
      std::make_shared<arrow::compute::CountOptions>(arrow::compute::CountOptions::CountMode::ALL);
   arrow::compute::Aggregate aggregate{
      "hash_count_all", count_options, std::vector<arrow::FieldRef>{}, COUNT_FIELD
   };

   std::vector<arrow::FieldRef> field_refs;
   for (auto group_by_field : group_by_fields_identifiers) {
      field_refs.emplace_back(arrow::FieldRef{group_by_field.name});
   }
   arrow::acero::AggregateNodeOptions aggregate_node_options({aggregate}, field_refs);
   ARROW_ASSIGN_OR_RAISE(
      node,
      arrow::acero::MakeExecNode("aggregate", arrow_plan.get(), {node}, aggregate_node_options)
   );

   ARROW_ASSIGN_OR_RAISE(node, addOrderingNodes(arrow_plan.get(), node, table->schema));

   ARROW_ASSIGN_OR_RAISE(node, addLimitAndOffsetNode(arrow_plan.get(), node));

   ARROW_ASSIGN_OR_RAISE(node, addZstdDecompressNode(arrow_plan.get(), node, table->schema));

   return QueryPlan::makeQueryPlan(arrow_plan, node);
}

std::vector<schema::ColumnIdentifier> Aggregated::getOutputSchema(
   const schema::TableSchema& table_schema
) const {
   std::vector<schema::ColumnIdentifier> fields =
      columnNamesToFields(this->group_by_fields, table_schema);
   fields.emplace_back(COUNT_FIELD, schema::ColumnType::INT);
   return fields;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Aggregated>& action) {
   const std::vector<std::string> group_by_fields =
      json.value("groupByFields", std::vector<std::string>());
   action = std::make_unique<Aggregated>(group_by_fields);
}

}  // namespace silo::query_engine::actions
