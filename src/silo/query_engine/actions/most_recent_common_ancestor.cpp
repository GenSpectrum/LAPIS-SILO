#include "silo/query_engine/actions/most_recent_common_ancestor.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <arrow/acero/options.h>
#include <arrow/compute/exec.h>
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>

#include "silo/common/phylo_tree.h"
#include "silo/common/tree_node_id.h"
#include "silo/config/database_config.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/exec_node/json_value_type_array_builder.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column_group.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {
using silo::common::MRCAResponse;
using silo::common::TreeNodeId;
using silo::schema::ColumnType;

MostRecentCommonAncestor::MostRecentCommonAncestor(
   std::string column_name,
   bool print_nodes_not_in_tree
)
    : column_name(std::move(column_name)),
      print_nodes_not_in_tree(print_nodes_not_in_tree) {}

using silo::query_engine::filter::operators::Operator;

void MostRecentCommonAncestor::validateOrderByFields(const schema::TableSchema& /*table_schema*/)
   const {
   const std::vector<std::string_view> fields{"mrcaNode", "missingNodeCount", "missingFromTree"};
   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         std::ranges::any_of(
            fields, [&](const std::string_view& result_field) { return result_field == field.name; }
         ),
         "OrderByField {} is not contained in the result of this operation. "
         "Allowed values are {}.",
         field.name,
         fmt::join(fields, ", ")
      );
   }
}

std::vector<std::string> GetNodeValues(
   std::shared_ptr<const storage::Table> table,
   const std::string& column_name,
   std::vector<CopyOnWriteBitmap>& bitmap_filter
) {
   std::vector<std::string> all_tree_node_ids;
   for (size_t i = 0; i < table->getNumberOfPartitions(); ++i) {
      const storage::TablePartition& table_partition = table->getPartition(i);
      const auto& string_column = table_partition.columns.string_columns.at(column_name);

      CopyOnWriteBitmap& filter = bitmap_filter[i];
      const size_t cardinality = filter->cardinality();
      if (cardinality == 0) {
         continue;
      }
      for (uint32_t row_in_table_partition : *filter) {
         auto value =
            string_column.lookupValue(string_column.getValues().at(row_in_table_partition));
         if (!value.empty()) {
            all_tree_node_ids.push_back(value);
         }
      }
   }
   return all_tree_node_ids;
}

arrow::Status addMRCAResponseToBuilder(
   std::vector<std::string>& all_node_ids,
   std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder>& output_builder,
   const storage::column::StringColumnMetadata* metadata,
   bool print_nodes_not_in_tree
) {
   MRCAResponse response = metadata->getMRCA(all_node_ids);

   if (auto builder = output_builder.find("mrcaNode"); builder != output_builder.end()) {
      ARROW_RETURN_NOT_OK(
         builder->second.insert(std::string(response.mrca_node_id.value_or(TreeNodeId{}).string))
      );
   }
   if (auto builder = output_builder.find("missingNodeCount"); builder != output_builder.end()) {
      ARROW_RETURN_NOT_OK(builder->second.insert(static_cast<int32_t>(response.not_in_tree.size()))
      );
   }
   if (auto builder = output_builder.find("missingFromTree"); builder != output_builder.end()) {
      ARROW_RETURN_NOT_OK(
         builder->second.insert(fmt::format("{}", fmt::join(response.not_in_tree, ",")))
      );
   }
   return arrow::Status::OK();
}

arrow::Result<QueryPlan> MostRecentCommonAncestor::toQueryPlanImpl(
   std::shared_ptr<const storage::Table> table,
   std::vector<CopyOnWriteBitmap> partition_filters,
   const config::QueryOptions& query_options
) const {
   CHECK_SILO_QUERY(
      table->schema.getColumn(column_name).has_value(),
      "Column '{}' not found in table schema",
      column_name
   );
   CHECK_SILO_QUERY(
      table->schema.getColumn(column_name).has_value() &&
         table->schema.getColumn(column_name).value().type == ColumnType::STRING,
      "MRCA action cannot be called on column '{}' as it is not a column of type STRING",
      column_name
   );
   const auto& optional_table_metadata =
      table->schema.getColumnMetadata<storage::column::StringColumnPartition>(column_name);
   CHECK_SILO_QUERY(
      optional_table_metadata.has_value() &&
         optional_table_metadata.value()->phylo_tree.has_value(),
      "MRCA action cannot be called on Column '{}' as it does not have a phylogenetic tree "
      "associated with it",
      column_name
   );
   auto table_metadata = optional_table_metadata.value();
   auto output_fields = getOutputSchema(table->schema);
   auto evaluated_partition_filters = partition_filters;

   auto column_name_to_evaluate = column_name;
   auto print_missing_nodes = print_nodes_not_in_tree;

   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      [table,
       column_name_to_evaluate,
       output_fields,
       evaluated_partition_filters,
       table_metadata,
       produced = false,
       print_missing_nodes]() mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (produced == true) {
         std::optional<arrow::ExecBatch> result = std::nullopt;
         return arrow::Future{result};
      }
      produced = true;

      std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder> output_builder;
      for (const auto& output_field : output_fields) {
         output_builder.emplace(
            output_field.name, exec_node::columnTypeToArrowType(output_field.type)
         );
      }

      auto all_node_ids = GetNodeValues(table, column_name_to_evaluate, evaluated_partition_filters);

      ARROW_RETURN_NOT_OK(
         addMRCAResponseToBuilder(all_node_ids, output_builder, table_metadata, print_missing_nodes)
      );

      // Order of result_columns is relevant as it needs to be consistent with vector in schema
      std::vector<arrow::Datum> result_columns;
      for (const auto& output_field : output_fields) {
         if (auto array_builder = output_builder.find(output_field.name);
             array_builder != output_builder.end()) {
            arrow::Datum datum;
            ARROW_ASSIGN_OR_RAISE(datum, array_builder->second.toDatum());
            result_columns.push_back(std::move(datum));
         }
      }
      ARROW_ASSIGN_OR_RAISE(
         std::optional<arrow::ExecBatch> result, arrow::ExecBatch::Make(result_columns)
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

std::vector<schema::ColumnIdentifier> MostRecentCommonAncestor::getOutputSchema(
   const schema::TableSchema& table_schema
) const {
   std::vector<schema::ColumnIdentifier> fields;
   fields.emplace_back("mrcaNode", schema::ColumnType::STRING);
   fields.emplace_back("missingNodeCount", schema::ColumnType::INT);
   if (print_nodes_not_in_tree) {
      fields.emplace_back("missingFromTree", schema::ColumnType::STRING);
   }
   return fields;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<MostRecentCommonAncestor>& action) {
   const bool print_nodes_not_in_tree = json.value("printNodesNotInTree", false);

   CHECK_SILO_QUERY(
      json.contains("columnName"),
      "error: 'columnName' field is required in MostRecentCommonAncestor action"
   );

   std::string column_name = json["columnName"].get<std::string>();
   action = std::make_unique<MostRecentCommonAncestor>(column_name, print_nodes_not_in_tree);
}

}  // namespace silo::query_engine::actions
