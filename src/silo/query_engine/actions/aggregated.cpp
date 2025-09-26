#include "silo/query_engine/actions/aggregated.h"

#include <optional>
#include <utility>
#include <vector>

#include <arrow/acero/options.h>
#include <arrow/compute/api.h>
#include <arrow/compute/expression.h>
#include <nlohmann/json.hpp>

#include "evobench/evobench.hpp"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/table_scan.h"
#include "silo/storage/table.h"

using silo::query_engine::CopyOnWriteBitmap;

namespace {
using silo::query_engine::actions::GroupByField;

const std::string GROUP_BY_FIELDS_FIELD_NAME = "groupByFields";

std::vector<silo::schema::ColumnIdentifier> bindGroupByFields(
   const silo::schema::TableSchema& schema,
   const std::vector<GroupByField>& group_by_fields
) {
   std::vector<silo::schema::ColumnIdentifier> group_by_metadata;
   for (const GroupByField& group_by_field : group_by_fields) {
      auto column = schema.getColumn(group_by_field.name);
      CHECK_SILO_QUERY(
         column.has_value(), "Metadata field '{}' to group by not found", group_by_field.name
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

Aggregated::Aggregated(std::vector<std::string> group_by_fields_) {
   group_by_fields.reserve(group_by_fields_.size());
   for (auto& field : group_by_fields_) {
      group_by_fields.emplace_back(std::move(field));
   }
}

void Aggregated::validateOrderByFields(const schema::TableSchema& schema) const {
   const std::vector<silo::schema::ColumnIdentifier> field_identifiers =
      bindGroupByFields(schema, group_by_fields);

   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         field.name == COUNT_FIELD || std::ranges::any_of(
                                         field_identifiers,
                                         [&](const silo::schema::ColumnIdentifier& metadata) {
                                            return metadata.name == field.name;
                                         }
                                      ),
         "The orderByField '{}' cannot be ordered by, as it does not appear in the {}.",
         field.name,
         GROUP_BY_FIELDS_FIELD_NAME
      );
   }
}

arrow::Result<QueryPlan> Aggregated::toQueryPlanImpl(
   std::shared_ptr<const storage::Table> table,
   std::vector<CopyOnWriteBitmap> partition_filters,
   const config::QueryOptions& query_options,
   std::string_view request_id
) const {
   EVOBENCH_SCOPE("Aggregated", "toQueryPlanImpl");
   if (group_by_fields.empty()) {
      return makeAggregateWithoutGrouping(table, partition_filters, query_options, request_id);
   } else {
      return makeAggregateWithGrouping(table, partition_filters, query_options, request_id);
   }
}

arrow::Result<QueryPlan> Aggregated::makeAggregateWithoutGrouping(
   std::shared_ptr<const storage::Table> table,
   std::vector<CopyOnWriteBitmap> partition_filters,
   const config::QueryOptions& /*query_options*/,
   std::string_view request_id
) const {
   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      [table, partition_filters, produced = false](
      ) mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (produced == true) {
         std::optional<arrow::ExecBatch> result = std::nullopt;
         return arrow::Future{result};
      }
      produced = true;

      int32_t result_count = 0;

      for (const auto& partition_filter : partition_filters) {
         result_count += partition_filter->cardinality();
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

   return QueryPlan::makeQueryPlan(arrow_plan, node, request_id);
}

arrow::Result<QueryPlan> Aggregated::makeAggregateWithGrouping(
   std::shared_ptr<const storage::Table> table,
   std::vector<CopyOnWriteBitmap> partition_filters,
   const config::QueryOptions& query_options,
   std::string_view request_id
) const {
   ARROW_ASSIGN_OR_RAISE(auto arrow_plan, arrow::acero::ExecPlan::Make());

   std::vector<schema::ColumnIdentifier> group_by_fields_identifiers =
      bindGroupByFields(table->schema, group_by_fields);

   arrow::acero::ExecNode* node;
   ARROW_ASSIGN_OR_RAISE(
      node,
      exec_node::makeTableScan(
         arrow_plan.get(),
         group_by_fields_identifiers,
         partition_filters,
         table,
         query_options.materialization_cutoff
      )
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

   return QueryPlan::makeQueryPlan(arrow_plan, node, request_id);
}

std::vector<schema::ColumnIdentifier> Aggregated::getOutputSchema(
   const schema::TableSchema& table_schema
) const {
   std::vector<schema::ColumnIdentifier> fields = bindGroupByFields(table_schema, group_by_fields);
   fields.emplace_back(COUNT_FIELD, schema::ColumnType::INT64);
   return fields;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Aggregated>& action) {
   std::vector<std::string> group_by_fields;
   if (json.contains(GROUP_BY_FIELDS_FIELD_NAME)) {
      CHECK_SILO_QUERY(
         json[GROUP_BY_FIELDS_FIELD_NAME].is_array(),
         "{} must be an array",
         GROUP_BY_FIELDS_FIELD_NAME
      );
      for (const auto& element : json[GROUP_BY_FIELDS_FIELD_NAME]) {
         CHECK_SILO_QUERY(
            element.is_string(),
            "{} is not a valid entry in {}. Expected type string, got {}",
            element.dump(),
            GROUP_BY_FIELDS_FIELD_NAME,
            element.type_name()
         );
         group_by_fields.emplace_back(element.get<std::string>());
      }
   }
   action = std::make_unique<Aggregated>(Action::deduplicateOrderPreserving(group_by_fields));
}

}  // namespace silo::query_engine::actions
