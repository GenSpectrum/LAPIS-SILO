#pragma once

#include <utility>

#include "silo/query_engine/operator_visitor.h"
#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/order_by_node.h"
#include "silo/query_engine/operators/project_node.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/operators/schema_node.h"
#include "silo/query_engine/operators/union_all_node.h"
#include "silo/query_engine/operators/unresolved_insertions_node.h"
#include "silo/query_engine/operators/unresolved_most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/unresolved_mutations_node.h"
#include "silo/query_engine/operators/unresolved_phylo_subtree_node.h"
#include "silo/query_engine/operators/zstd_decompress_node.h"

namespace silo::query_engine::optimizer {

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
   /// Customization point for constructing the pass instance used by `run()`. The default
   /// default-constructs `Derived`. A pass that needs to seed state from the root node (e.g.
   /// from its output schema) overrides this instead of reimplementing `run()`.
   static Derived makePass(const operators::QueryNodePtr& /*node*/) { return Derived{}; }

   /// Entry point: walks `node` with a pass obtained from `makePass(node)` and returns the
   /// (possibly replaced) root.
   static operators::QueryNodePtr run(operators::QueryNodePtr node) {
      Derived pass = Derived::makePass(node);
      pass.propagateToNode(node);
      return node;
   }

   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::FilterNode& node) {
      propagateToNode(node.child);
      return nullptr;
   }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::AggregateNode& node) {
      propagateToNode(node.child);
      return nullptr;
   }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::ProjectNode& node) {
      propagateToNode(node.child);
      return nullptr;
   }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::MapNode& node) {
      propagateToNode(node.child);
      return nullptr;
   }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::OrderByNode& node) {
      propagateToNode(node.child);
      return nullptr;
   }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::FetchNode& node) {
      propagateToNode(node.child);
      return nullptr;
   }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::ZstdDecompressNode& node) {
      propagateToNode(node.child);
      return nullptr;
   }

   template <typename SymbolType>
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::UnresolvedMutationsNode<SymbolType>& node) {
      propagateToNode(node.child);
      return nullptr;
   }
   template <typename SymbolType>
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::UnresolvedInsertionsNode<SymbolType>& node) {
      propagateToNode(node.child);
      return nullptr;
   }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::UnresolvedMostRecentCommonAncestorNode& node) {
      propagateToNode(node.child);
      return nullptr;
   }
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::UnresolvedPhyloSubtreeNode& node) {
      propagateToNode(node.child);
      return nullptr;
   }

   // Default propagation for the two-child UnionAll operator. Both branches are
   // walked by the same pass instance, so this default is only correct for
   // stateless passes. A stateful pass MUST override this.
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::UnionAllNode& node) {
      propagateToNode(node.left);
      propagateToNode(node.right);
      return nullptr;
   }

   template <typename T>
   operators::QueryNodePtr operator()(T& /*node*/) {
      return nullptr;
   }

  protected:
   /// Visit `node` with this pass once, replacing it if the pass returns a non-null
   /// replacement. Handlers are responsible for recursing into their own children
   /// before returning a replacement, so a single visit per node is sufficient.
   // NOLINTNEXTLINE(misc-no-recursion)
   void propagateToNode(operators::QueryNodePtr& node) {
      auto& derived = static_cast<Derived&>(*this);
      if (auto replacement = operators::visit(*node, derived)) {
         node = std::move(replacement);
      }
   }
};

}  // namespace silo::query_engine::optimizer
