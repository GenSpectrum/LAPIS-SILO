#include "silo/query_engine/expressions/phylo_child_filter.h"

#include <optional>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include "silo/common/panic.h"
#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/bitmap_producer.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"

namespace silo::query_engine::expressions {

PhyloChildFilter::PhyloChildFilter(std::string column_name, std::string internal_node)
    : column_name(std::move(column_name)),
      internal_node(std::move(internal_node)) {}

std::vector<schema::ColumnIdentifier> PhyloChildFilter::freeIUs() const {
   return {{column_name, schema::ColumnType::BOOL}};
}

std::string PhyloChildFilter::toString() const {
   return fmt::format("column {} phylo_child_of {}", column_name, internal_node);
};

namespace {
std::unique_ptr<filter::operators::Operator> createMatchingBitmap(
   const storage::column::StringColumn& string_column,
   const std::string& internal_node,
   storage::column::RowLayout row_layout
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
   return std::make_unique<filter::operators::BitmapProducer>(
      [&string_column, internal_tree_node]() {
         roaring::Roaring result_bitmap = string_column.getDescendants(internal_tree_node.value());
         return CopyOnWriteBitmap(std::move(result_bitmap));
      },
      std::move(row_layout)
   );
}

}  // namespace

std::unique_ptr<Expression> PhyloChildFilter::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<PhyloChildFilter>(column_name, internal_node);
}

std::unique_ptr<filter::operators::Operator> PhyloChildFilter::compile(const storage::Table& table
) const {
   CHECK_SILO_QUERY(
      table.schema->getColumn(column_name).has_value(),
      "The database does not contain the column '{}'",
      column_name
   );
   CHECK_SILO_QUERY(
      table.columns.string_columns.contains(column_name),
      "The column '{}' is not of type string",
      column_name
   );

   SILO_ASSERT(table.columns.string_columns.contains(column_name));
   const auto& string_column = table.columns.string_columns.at(column_name);
   return createMatchingBitmap(string_column, internal_node, table.row_layout);
}

}  // namespace silo::query_engine::expressions
