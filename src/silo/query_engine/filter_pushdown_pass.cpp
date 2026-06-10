#include "silo/query_engine/filter_pushdown_pass.h"

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/expressions/and.h"
#include "silo/query_engine/operator_visitor.h"
#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/insertions_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/mutations_node.h"
#include "silo/query_engine/operators/order_by_node.h"
#include "silo/query_engine/operators/phylo_subtree_node.h"
#include "silo/query_engine/operators/project_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/operators/union_all_node.h"
#include "silo/query_engine/operators/zstd_decompress_node.h"

namespace silo::query_engine {

namespace {

// NOLINTNEXTLINE(misc-no-recursion)
void applyToNode(operators::QueryNodePtr& node, FilterPushdownPass& pass) {
   while (auto replacement = operators::visit(*node, pass)) {
      node = std::move(replacement);
   }
}

}  // namespace

operators::QueryNodePtr FilterPushdownPass::run(operators::QueryNodePtr node) {
   FilterPushdownPass pass;
   applyToNode(node, pass);
   return node;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::FilterNode& node) {
   current_filters.push_back(std::move(node.filter));
   auto child = std::move(node.child);
   applyToNode(child, *this);
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
   applyToNode(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::OrderByNode& node) {
   applyToNode(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::FetchNode& node) {
   applyToNode(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::ProjectNode& node) {
   applyToNode(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::MapNode& node) {
   applyToNode(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::ZstdDecompressNode& node) {
   applyToNode(node.child, *this);
   return nullptr;
}

template <typename SymbolType>
// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::UnresolvedMutationsNode<SymbolType>& node
) {
   applyToNode(node.child, *this);
   return nullptr;
}

template <typename SymbolType>
// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::UnresolvedInsertionsNode<SymbolType>& node
) {
   applyToNode(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::UnresolvedMostRecentCommonAncestorNode& node
) {
   applyToNode(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::UnresolvedPhyloSubtreeNode& node
) {
   applyToNode(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::UnionAllNode& node) {
   // Filters cannot be pushed across a union all boundary.
   // Each child is optimized independently with a fresh pass.
   for (auto& child : node.children) {
      FilterPushdownPass child_pass;
      applyToNode(child, child_pass);
   }
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
