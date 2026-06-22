#pragma once

#include "silo/query_engine/operator_visitor.h"
#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/order_by_node.h"
#include "silo/query_engine/operators/project_node.h"
#include "silo/query_engine/operators/zstd_decompress_node.h"

namespace silo::query_engine {

/// CRTP base class for query-plan optimization passes.
/// Automatically propagates the pass through pipelined (single-child, non-transforming)
/// operators: FetchNode, MapNode, ProjectNode, ZstdDecompressNode, OrderByNode.
/// Derived passes only need to implement handlers for nodes they specifically care about.
template <typename Derived>
class PipelinePassBase {
  protected:
   /// Repeatedly visits `node` with this pass, replacing it each time the pass
   /// returns a non-null replacement node, until the pass returns nullptr.
   // NOLINTNEXTLINE(misc-no-recursion)
   void propagateToChild(operators::QueryNodePtr& node) {
      auto& derived = static_cast<Derived&>(*this);
      while (auto replacement = operators::visit(*node, derived)) {
         node = std::move(replacement);
      }
   }

  public:
   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::FetchNode& node) {
      propagateToChild(node.child);
      return nullptr;
   }

   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::MapNode& node) {
      propagateToChild(node.child);
      return nullptr;
   }

   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::ProjectNode& node) {
      propagateToChild(node.child);
      return nullptr;
   }

   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::ZstdDecompressNode& node) {
      propagateToChild(node.child);
      return nullptr;
   }

   // NOLINTNEXTLINE(misc-no-recursion)
   operators::QueryNodePtr operator()(operators::OrderByNode& node) {
      propagateToChild(node.child);
      return nullptr;
   }
};

}  // namespace silo::query_engine
