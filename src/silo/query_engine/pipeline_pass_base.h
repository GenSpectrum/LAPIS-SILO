#pragma once

#include "silo/query_engine/operator_visitor.h"
#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/order_by_node.h"
#include "silo/query_engine/operators/project_node.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/operators/union_all_node.h"
#include "silo/query_engine/operators/unresolved_insertions_node.h"
#include "silo/query_engine/operators/unresolved_most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/unresolved_mutations_node.h"
#include "silo/query_engine/operators/unresolved_phylo_subtree_node.h"
#include "silo/query_engine/operators/zstd_decompress_node.h"

namespace silo::query_engine {

/// CRTP base for optimization passes that walk the QueryNode tree.
///
/// Provides default propagation for every pipelined (single-child) operator: the
/// pass is applied to the operator's child and the operator itself is kept in place.
/// `UnionAllNode` (two children) propagates into both branches. This removes the
/// boilerplate of writing one identical `operator()` per pipeline operator in each
/// pass.
///
/// A derived pass overrides only the operators that need custom behaviour. It must
/// pull the defaults into its own scope so they are not hidden by its overrides:
///
/// ```cpp
/// class MyPass : public PipelinePassBase<MyPass> {
///   public:
///    using PipelinePassBase<MyPass>::operator();
///    operators::QueryNodePtr operator()(operators::FilterNode& node);  // custom
/// };
/// ```
///
/// Replacing a node: returning a non-null `QueryNodePtr` from an `operator()` replaces
/// the visited node with the returned one; `nullptr` keeps the node in place.
template <typename Derived>
class PipelinePassBase {
   // Private constructor + friend Derived enforces correct CRTP usage: only `Derived`
   // can instantiate this base, preventing accidental `class X : PipelinePassBase<Y>`.
   PipelinePassBase() = default;
   friend Derived;

  public:
   /// Repeatedly visit `node` (replacing it while a replacement is produced), then
   /// recurse into children via the per-operator handlers.
   // NOLINTNEXTLINE(misc-no-recursion)
   static void applyToNode(operators::QueryNodePtr& node, Derived& pass) {
      while (auto replacement = operators::visit(*node, pass)) {
         node = std::move(replacement);
      }
   }

   static operators::QueryNodePtr run(operators::QueryNodePtr node) {
      Derived pass;
      applyToNode(node, pass);
      return node;
   }

   // Default propagation for single-child pipeline operators.
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::FilterNode& node) { return propagate(node.child); }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::AggregateNode& node) {
      return propagate(node.child);
   }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::ProjectNode& node) {
      return propagate(node.child);
   }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::MapNode& node) { return propagate(node.child); }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::OrderByNode& node) {
      return propagate(node.child);
   }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::FetchNode& node) { return propagate(node.child); }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::ZstdDecompressNode& node) {
      return propagate(node.child);
   }

   // Default propagation for unresolved placeholder operators (also single-child).
   template <typename SymbolType>
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::UnresolvedMutationsNode<SymbolType>& node) {
      return propagate(node.child);
   }
   template <typename SymbolType>
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::UnresolvedInsertionsNode<SymbolType>& node) {
      return propagate(node.child);
   }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::UnresolvedMostRecentCommonAncestorNode& node) {
      return propagate(node.child);
   }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::UnresolvedPhyloSubtreeNode& node) {
      return propagate(node.child);
   }

   // Default propagation for the two-child UnionAll operator.
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::UnionAllNode& node) {
      applyToNode(node.left, self());
      applyToNode(node.right, self());
      return nullptr;
   }

   // Leaf nodes (TableScanNode, MutationsNode, etc.) terminate the walk.
   template <typename T>
   operators::QueryNodePtr operator()(T& /*node*/) {
      return nullptr;
   }

  protected:
   Derived& self() { return static_cast<Derived&>(*this); }

   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr propagate(operators::QueryNodePtr& child) {
      applyToNode(child, self());
      return nullptr;
   }
};

}  // namespace silo::query_engine
