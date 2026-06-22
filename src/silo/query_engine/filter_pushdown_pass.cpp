#include "silo/query_engine/filter_pushdown_pass.h"

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/expressions/and.h"
#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/insertions_node.h"
#include "silo/query_engine/operators/most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/mutations_node.h"
#include "silo/query_engine/operators/phylo_subtree_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/operators/union_all_node.h"

namespace silo::query_engine {

operators::QueryNodePtr FilterPushdownPass::run(operators::QueryNodePtr node) {
   FilterPushdownPass pass;
   pass.propagateToChild(node);
   return node;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::FilterNode& node) {
   current_filters.push_back(std::move(node.filter));
   auto child = std::move(node.child);
   propagateToChild(child);
   return child;
}

operators::QueryNodePtr FilterPushdownPass::operator()(operators::TableScanNode& node) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<expressions::And>(std::move(current_filters));
   return nullptr;
}

operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::MutationsNode<silo::Nucleotide>& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<expressions::And>(std::move(current_filters));
   return nullptr;
}

operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::MutationsNode<silo::AminoAcid>& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<expressions::And>(std::move(current_filters));
   return nullptr;
}
operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::InsertionsNode<silo::Nucleotide>& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<expressions::And>(std::move(current_filters));
   return nullptr;
}
operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::InsertionsNode<silo::AminoAcid>& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<expressions::And>(std::move(current_filters));
   return nullptr;
}
operators::QueryNodePtr FilterPushdownPass::operator()(operators::PhyloSubtreeNode& node) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<expressions::And>(std::move(current_filters));
   return nullptr;
}
operators::QueryNodePtr FilterPushdownPass::operator()(operators::MostRecentCommonAncestorNode& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<expressions::And>(std::move(current_filters));
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::AggregateNode& node) {
   propagateToChild(node.child);
   return nullptr;
}

template <typename SymbolType>
// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::UnresolvedMutationsNode<SymbolType>& node
) {
   propagateToChild(node.child);
   return nullptr;
}

template <typename SymbolType>
// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::UnresolvedInsertionsNode<SymbolType>& node
) {
   propagateToChild(node.child);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::UnresolvedMostRecentCommonAncestorNode& node
) {
   propagateToChild(node.child);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::UnresolvedPhyloSubtreeNode& node
) {
   propagateToChild(node.child);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::UnionAllNode& node) {
   // Push parent filters into both children. Clone for right, move originals into left.
   FilterPushdownPass right_pass;
   for (const auto& filter : current_filters) {
      right_pass.current_filters.push_back(filter->clone());
   }
   FilterPushdownPass left_pass;
   left_pass.current_filters = std::move(current_filters);

   left_pass.propagateToChild(node.left);
   right_pass.propagateToChild(node.right);
   return nullptr;
}

template operators::QueryNodePtr FilterPushdownPass::operator()(operators::UnresolvedMutationsNode<
                                                                silo::Nucleotide>&);
template operators::QueryNodePtr FilterPushdownPass::operator()(operators::UnresolvedMutationsNode<
                                                                silo::AminoAcid>&);
template operators::QueryNodePtr FilterPushdownPass::operator()(operators::UnresolvedInsertionsNode<
                                                                silo::Nucleotide>&);
template operators::QueryNodePtr FilterPushdownPass::operator()(operators::UnresolvedInsertionsNode<
                                                                silo::AminoAcid>&);

}  // namespace silo::query_engine
