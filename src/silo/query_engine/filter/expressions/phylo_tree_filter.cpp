#include "silo/query_engine/filter/expressions/phylo_tree_filter.h"

#include <optional>
#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/common/panic.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/bitmap_producer.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

PhyloChildFilter::PhyloChildFilter(std::string column_name, std::string internal_node)
    : column_name(std::move(column_name)),
      internal_node(std::move(internal_node)) {}

std::string PhyloChildFilter::toString() const {
   return fmt::format("column {} phylo_child_of {}", column_name, internal_node);
};

namespace {
std::unique_ptr<silo::query_engine::filter::operators::Operator> createMatchingBitmap(
   const storage::column::StringColumnPartition& string_column,
   const std::string& internal_node,
   size_t row_count
) {
   CHECK_SILO_QUERY(
      string_column.metadata->phylo_tree.has_value(),
      "Phylotree filter cannot be called on Column '{}' as it does not have a phylogenetic tree "
      "associated with it",
      string_column.metadata->column_name
   );
   auto internal_tree_node = string_column.metadata->phylo_tree->getTreeNodeId(internal_node);
   CHECK_SILO_QUERY(
      internal_tree_node.has_value(),
      "The node '{}' does not exist in the phylogenetic tree of column '{}'",
      internal_node,
      string_column.metadata->column_name
   );
   return std::make_unique<operators::BitmapProducer>(
      [&string_column, internal_tree_node]() {
         roaring::Roaring result_bitmap = string_column.getDescendants(internal_tree_node.value());
         return CopyOnWriteBitmap(std::move(result_bitmap));
      },
      row_count
   );
}

}  // namespace

std::unique_ptr<Expression> PhyloChildFilter::rewrite(
   const storage::Table& /*table*/,
   const storage::TablePartition& /*table_partition*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<PhyloChildFilter>(column_name, internal_node);
}

std::unique_ptr<silo::query_engine::filter::operators::Operator> PhyloChildFilter::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition
) const {
   CHECK_SILO_QUERY(
      table_partition.columns.string_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );

   SILO_ASSERT(table_partition.columns.string_columns.contains(column_name));
   const auto& string_column = table_partition.columns.string_columns.at(column_name);
   return createMatchingBitmap(string_column, internal_node, table_partition.sequence_count);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<PhyloChildFilter>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in an PhyloChildFilter expression"
   )
   CHECK_SILO_QUERY(
      json["column"].is_string(),
      "The field 'column' in an PhyloChildFilter expression needs to be a string"
   )
   CHECK_SILO_QUERY(
      json.contains("internalNode"),
      "The field 'internalNode' is required in an PhyloChildFilter expression"
   )
   CHECK_SILO_QUERY(
      json["internalNode"].is_string(),
      "The field 'internalNode' in an PhyloChildFilter expression needs to be a string"
   )
   filter = std::make_unique<PhyloChildFilter>(
      json["column"].get<std::string>(), json["internalNode"].get<std::string>()
   );
}

}  // namespace silo::query_engine::filter::expressions
