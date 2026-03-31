#include "silo/query_engine/operators/phylo_subtree_node.h"

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include "silo/common/phylo_tree.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/exec_node/schema_output_builder.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/operators/compute_filter.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/table.h"

namespace {

struct NodeValuesResult {
   std::unordered_set<std::string> node_values;
   uint32_t missing_node_count = 0;
};

NodeValuesResult getNodeValuesFromTable(
   const silo::storage::Table& table,
   const std::string& column_name,
   silo::query_engine::CopyOnWriteBitmap& bitmap_filter
) {
   const size_t num_rows = bitmap_filter.getConstReference().cardinality();
   std::unordered_set<std::string> all_tree_node_ids;
   uint32_t num_empty = 0;
   all_tree_node_ids.reserve(num_rows);

   const auto& string_column = table.columns.string_columns.at(column_name);

   for (const uint32_t row_in_table : bitmap_filter.getConstReference()) {
      if (!string_column.isNull(row_in_table)) {
         auto value = string_column.getValueString(row_in_table);
         all_tree_node_ids.insert(value);
      } else {
         ++num_empty;
      }
   }
   return NodeValuesResult{
      .node_values = std::move(all_tree_node_ids), .missing_node_count = num_empty
   };
}

}  // namespace

namespace silo::query_engine::operators {

PhyloSubtreeNode::PhyloSubtreeNode(
   std::shared_ptr<storage::Table> table,
   std::unique_ptr<filter::expressions::Expression> filter,
   std::string column_name,
   bool print_nodes_not_in_tree,
   bool contract_unary_nodes
)
    : table(std::move(table)),
      filter(std::move(filter)),
      column_name(std::move(column_name)),
      print_nodes_not_in_tree(print_nodes_not_in_tree),
      contract_unary_nodes(contract_unary_nodes) {}

std::vector<schema::ColumnIdentifier> PhyloSubtreeNode::getOutputSchema() const {
   std::vector<schema::ColumnIdentifier> output_fields;
   output_fields.emplace_back("missingNodeCount", schema::ColumnType::INT32);
   if (print_nodes_not_in_tree) {
      output_fields.emplace_back("missingFromTree", schema::ColumnType::STRING);
   }
   output_fields.emplace_back("subtreeNewick", schema::ColumnType::STRING);
   return output_fields;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Result<PartialArrowPlan> PhyloSubtreeNode::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
   const config::QueryOptions& /*query_options*/
) const {
   auto bitmap_filter = computeFilter(filter, *table);

   CHECK_SILO_QUERY(
      table->schema->getColumn(column_name).has_value(),
      "Column '{}' not found in table schema",
      column_name
   );
   CHECK_SILO_QUERY(
      table->schema->getColumn(column_name).value().type == schema::ColumnType::STRING,
      "PhyloSubtree action cannot be called on column '{}' as it is not a column of type STRING",
      column_name
   );
   const auto& optional_table_metadata =
      table->schema->getColumnMetadata<storage::column::StringColumn>(column_name);
   CHECK_SILO_QUERY(
      optional_table_metadata.has_value() &&
         optional_table_metadata.value()->phylo_tree.has_value(),
      "PhyloSubtree action cannot be called on Column '{}' as it does not have a "
      "phylogenetic tree associated with it",
      column_name
   );
   const auto& phylo_tree = optional_table_metadata.value()->phylo_tree.value();

   const std::vector<schema::ColumnIdentifier> output_fields = getOutputSchema();

   auto table_handle = table;
   const auto column_name_copy = column_name;
   const bool contract = contract_unary_nodes;

   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      [table_handle,
       column_name_copy,
       output_fields,
       bitmap_filter = std::move(bitmap_filter),
       &phylo_tree,
       contract,
       already_produced = false]() mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (already_produced) {
         const std::optional<arrow::ExecBatch> result = std::nullopt;
         return arrow::Future{result};
      }
      already_produced = true;

      exec_node::SchemaOutputBuilder output_builder{output_fields};

      auto node_vals = getNodeValuesFromTable(*table_handle, column_name_copy, bitmap_filter);

      common::NewickResponse newick_resp =
         phylo_tree.toNewickString(node_vals.node_values, contract);
      auto missing_count =
         static_cast<int32_t>(node_vals.missing_node_count + newick_resp.not_in_tree.size());

      ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput("subtreeNewick", [&]() {
         return newick_resp.newick_string;
      }));
      ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput("missingNodeCount", [&]() {
         return missing_count;
      }));
      ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput("missingFromTree", [&]() {
         return fmt::format("{}", fmt::join(newick_resp.not_in_tree, ","));
      }));

      ARROW_ASSIGN_OR_RAISE(
         const std::vector<arrow::Datum> result_columns, output_builder.finish()
      );
      ARROW_ASSIGN_OR_RAISE(
         const std::optional<arrow::ExecBatch> result, arrow::ExecBatch::Make(result_columns)
      );
      return arrow::Future{result};
   };

   ARROW_ASSIGN_OR_RAISE(auto arrow_plan, arrow::acero::ExecPlan::Make());

   const arrow::acero::SourceNodeOptions options{
      exec_node::columnsToArrowSchema(output_fields),
      std::move(producer),
      arrow::Ordering::Implicit()
   };
   ARROW_ASSIGN_OR_RAISE(
      auto node, arrow::acero::MakeExecNode("source", arrow_plan.get(), {}, options)
   );

   return PartialArrowPlan{.top_node = node, .plan = arrow_plan};
}

}  // namespace silo::query_engine::operators
