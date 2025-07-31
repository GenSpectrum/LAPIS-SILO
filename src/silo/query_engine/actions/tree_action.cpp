#include "silo/query_engine/actions/tree_action.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <arrow/acero/options.h>
#include <arrow/compute/exec.h>
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>

#include "evobench/evobench.hpp"
#include "silo/common/phylo_tree.h"
#include "silo/common/tree_node_id.h"
#include "silo/config/database_config.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column_group.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {
using silo::schema::ColumnType;

TreeAction::TreeAction(std::string column_name, bool print_nodes_not_in_tree)
    : column_name(std::move(column_name)),
      print_nodes_not_in_tree(print_nodes_not_in_tree) {}

using silo::query_engine::filter::operators::Operator;

void TreeAction::validateOrderByFields(const schema::TableSchema& schema) const {
   std::vector<std::string_view> allowed{myResultFieldName(), "missingNodeCount"};
   if (print_nodes_not_in_tree) {
      allowed.push_back("missingFromTree");
   }

   for (const auto& field : order_by_fields) {
      bool ok = std::ranges::any_of(allowed, [&](std::string_view f) { return f == field.name; });
      CHECK_SILO_QUERY(
         ok,
         "OrderByField {} is not contained in the result of this operation. "
         "Allowed values are {}.",
         field.name,
         fmt::join(allowed, ", ")
      );
   }
}

std::unordered_set<std::string> TreeAction::getNodeValues(
   std::shared_ptr<const storage::Table> table,
   const std::string& column_name,
   std::vector<CopyOnWriteBitmap>& bitmap_filter
) const {
   size_t num_rows = 0;
   for (const auto& filter : bitmap_filter) {
      num_rows += filter->cardinality();
   }

   std::unordered_set<std::string> all_tree_node_ids;
   all_tree_node_ids.reserve(num_rows);
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
            all_tree_node_ids.insert(value);
         }
      }
   }
   return all_tree_node_ids;
}

arrow::Result<QueryPlan> TreeAction::toQueryPlanImpl(
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
      "{} action cannot be called on column '{}' as it is not a column of type STRING",
      getType(),
      column_name
   );
   const auto& optional_table_metadata =
      table->schema.getColumnMetadata<storage::column::StringColumnPartition>(column_name);
   CHECK_SILO_QUERY(
      optional_table_metadata.has_value() &&
         optional_table_metadata.value()->phylo_tree.has_value(),
      "{} action cannot be called on Column '{}' as it does not have a phylogenetic tree "
      "associated with it",
      getType(),
      column_name
   );
   const auto& phylo_tree = optional_table_metadata.value()->phylo_tree.value();
   auto output_fields = getOutputSchema(table->schema);
   auto evaluated_partition_filters = partition_filters;

   auto column_name_to_evaluate = column_name;
   auto print_missing_nodes = print_nodes_not_in_tree;

   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      [this,
       table,
       column_name_to_evaluate,
       output_fields,
       evaluated_partition_filters,
       &phylo_tree,
       produced = false,
       print_missing_nodes]() mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      EVOBENCH_SCOPE("TreeAction", "producer");
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

      auto all_node_ids =
         this->getNodeValues(table, column_name_to_evaluate, evaluated_partition_filters);

      ARROW_RETURN_NOT_OK(
         this->addResponseToBuilder(all_node_ids, output_builder, phylo_tree, print_missing_nodes)
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

}  // namespace silo::query_engine::actions
