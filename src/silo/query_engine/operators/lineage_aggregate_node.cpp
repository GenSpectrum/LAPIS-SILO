#include "silo/query_engine/operators/lineage_aggregate_node.h"

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <roaring/roaring.hh>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/builder.h>
#include <nlohmann/json.hpp>

#include "silo/common/panic.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/operators/compute_filter.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

LineageAggregateNode::LineageAggregateNode(
   std::shared_ptr<storage::Table> table,
   std::unique_ptr<expressions::Expression> filter,
   std::string column_name,
   common::RecombinantEdgeFollowingMode mode,
   std::string count_output_name
)
    : table(std::move(table)),
      filter(std::move(filter)),
      column_name(std::move(column_name)),
      mode(mode),
      count_output_name(std::move(count_output_name)) {}

std::vector<schema::ColumnIdentifier> LineageAggregateNode::getOutputSchema() const {
   return {
      {column_name, schema::ColumnType::STRING}, {count_output_name, schema::ColumnType::INT64}
   };
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Result<arrow::acero::ExecNode*> LineageAggregateNode::addToExecPlan(
   arrow::acero::ExecPlan& plan,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
   const config::QueryOptions& /*query_options*/
) const {
   CHECK_SILO_QUERY(
      table->schema->getColumn(column_name).has_value(),
      "The database does not contain the column '{}'",
      column_name
   );
   CHECK_SILO_QUERY(
      table->columns.indexed_string_columns.contains(column_name),
      "lineage aggregation cannot be applied to column '{}' as it is not of type indexed string",
      column_name
   );
   const auto& lineage_column = table->columns.indexed_string_columns.at(column_name);
   CHECK_SILO_QUERY(
      lineage_column.getLineageIndex().has_value(),
      "The database does not contain a lineage index for the column '{}'",
      column_name
   );

   auto bitmap_filter = computeFilter(filter, *table);
   const roaring::Roaring& filter_reference = bitmap_filter.getConstReference();

   const auto& lineage_index = lineage_column.getLineageIndex().value();
   const auto& lineage_tree = lineage_column.metadata->lineage_tree.value().lineage_tree;

   // Enumerate the canonical defined lineages. Canonical lineages occupy the id range
   // [0, numberOfLineages()); aliases are assigned ids beyond that range, so iterating this range
   // visits every defined lineage exactly once and never visits an alias. This makes the
   // alias-double-emit trap impossible by construction. The assert guards the shared-id-space
   // invariant so a future refactor that decouples the tree/dictionary id spaces fails loudly.
   const size_t number_of_lineages = lineage_tree.numberOfLineages();
   arrow::StringBuilder lineage_builder;
   arrow::Int64Builder count_builder;
   ARROW_RETURN_NOT_OK(lineage_builder.Reserve(static_cast<int64_t>(number_of_lineages)));
   ARROW_RETURN_NOT_OK(count_builder.Reserve(static_cast<int64_t>(number_of_lineages)));

   for (Idx lineage_id = 0; lineage_id < number_of_lineages; ++lineage_id) {
      SILO_ASSERT(lineage_tree.resolveAlias(lineage_id) == lineage_id);
      const std::string_view lineage_name = lineage_column.lookupValue(lineage_id);
      const std::optional<const roaring::Roaring*> subtree_bitmap =
         lineage_index.filterIncludingSublineages(lineage_id, mode);
      const int64_t count = subtree_bitmap.has_value()
                               ? static_cast<int64_t>(
                                    filter_reference.and_cardinality(*subtree_bitmap.value())
                                 )
                               : 0;
      ARROW_RETURN_NOT_OK(lineage_builder.Append(lineage_name));
      ARROW_RETURN_NOT_OK(count_builder.Append(count));
   }

   arrow::Datum lineage_datum;
   ARROW_ASSIGN_OR_RAISE(lineage_datum, lineage_builder.Finish());
   arrow::Datum count_datum;
   ARROW_ASSIGN_OR_RAISE(count_datum, count_builder.Finish());

   const std::vector<schema::ColumnIdentifier> output_fields = getOutputSchema();

   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      [lineage_datum = std::move(lineage_datum),
       count_datum = std::move(count_datum),
       already_produced = false]() mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (already_produced) {
         const std::optional<arrow::ExecBatch> result = std::nullopt;
         return arrow::Future{result};
      }
      already_produced = true;
      ARROW_ASSIGN_OR_RAISE(
         const std::optional<arrow::ExecBatch> result,
         arrow::ExecBatch::Make({lineage_datum, count_datum})
      );
      return arrow::Future{result};
   };

   const arrow::acero::SourceNodeOptions options{
      exec_node::columnsToArrowSchema(output_fields),
      std::move(producer),
      arrow::Ordering::Implicit()
   };
   return arrow::acero::MakeExecNode("source", &plan, {}, options);
}

nlohmann::json LineageAggregateNode::toJson() const {
   return {
      {"type", nodeKindToString(kind())},
      {"table", table->logTable()},
      {"filter", filter->toString()},
      {"columnName", column_name},
      {"recombinantFollowingMode", static_cast<int>(mode)},
      {"countOutputName", count_output_name},
   };
}

}  // namespace silo::query_engine::operators
