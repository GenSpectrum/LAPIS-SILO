#include "silo/query_engine/column_narrowing_pass.h"

#include "silo/query_engine/operator_visitor.h"
#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/order_by_node.h"
#include "silo/query_engine/operators/project_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/operators/unresolved_insertions_node.h"
#include "silo/query_engine/operators/unresolved_most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/unresolved_mutations_node.h"
#include "silo/query_engine/operators/unresolved_phylo_subtree_node.h"

namespace silo::query_engine {

namespace {
// NOLINTNEXTLINE(misc-no-recursion)
void applyToChild(operators::QueryNodePtr& child, ColumnNarrowingPass& pass) {
   if (auto new_child = operators::visit(*child, pass)) {
      child = std::move(new_child);
   }
}
}  // namespace

// NOLINTNEXTLINE(readability-convert-member-functions-to-static,misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::TableScanNode& node) {
   std::vector<schema::ColumnIdentifier> pruned;
   for (const auto& col : node.fields) {
      if (std::ranges::find(required, col) != required.end()) {
         pruned.push_back(col);
      }
   }
   node.fields = std::move(pruned);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::AggregateNode& node) {
   std::vector<schema::ColumnIdentifier> child_required;
   child_required.reserve(node.group_by_fields.size());
   for (const auto& field : node.group_by_fields) {
      child_required.push_back(field);
   }
   for (const auto& agg : node.aggregates) {
      if (agg.source_column.has_value()) {
         child_required.push_back(agg.source_column.value());
      }
   }

   if (child_required.empty()) {
      // COUNT(*) with no group-by: still need one column to drive the row stream.
      auto child_schema = node.child->getOutputSchema();
      SILO_ASSERT(!child_schema.empty());
      required = RequiredColumns{child_schema.front()};
   } else {
      required = std::move(child_required);
   }
   applyToChild(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::ProjectNode& node) {
   std::vector<schema::ColumnIdentifier> narrowed_fields;
   for (const auto& field : node.fields) {
      if (std::ranges::find(required, field) != required.end()) {
         narrowed_fields.push_back(field);
      }
   }
   node.fields = narrowed_fields;

   required = node.fields;
   applyToChild(node.child, *this);
   if (node.child->getOutputSchema() == node.fields) {
      return std::move(node.child);
   }
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::OrderByNode& node) {
   for (const auto& field : node.fields) {
      if (std::ranges::find(required, field.field) == required.end()) {
         required.push_back(field.field);
      }
   }
   applyToChild(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::FetchNode& node) {
   applyToChild(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::FilterNode& node) {
   applyToChild(node.child, *this);
   return nullptr;
}

template <typename SymbolType>
// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(
   operators::UnresolvedMutationsNode<SymbolType>& node
) {
   applyToChild(node.child, *this);
   return nullptr;
}

template <typename SymbolType>
// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(
   operators::UnresolvedInsertionsNode<SymbolType>& node
) {
   applyToChild(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(
   operators::UnresolvedMostRecentCommonAncestorNode& node
) {
   applyToChild(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::UnresolvedPhyloSubtreeNode& node
) {
   applyToChild(node.child, *this);
   return nullptr;
}

template operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::UnresolvedMutationsNode<
                                                                 silo::Nucleotide>&);
template operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::UnresolvedMutationsNode<
                                                                 silo::AminoAcid>&);
template operators::QueryNodePtr ColumnNarrowingPass::
operator()(operators::UnresolvedInsertionsNode<silo::Nucleotide>&);
template operators::QueryNodePtr ColumnNarrowingPass::
operator()(operators::UnresolvedInsertionsNode<silo::AminoAcid>&);

}  // namespace silo::query_engine
