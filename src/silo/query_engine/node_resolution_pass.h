#pragma once

#include <string_view>

#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/pipeline_pass_base.h"

namespace silo::query_engine::operators {
class AggregateNode;
template <typename SymbolType>
class UnresolvedMutationsNode;
template <typename SymbolType>
class UnresolvedInsertionsNode;
class UnresolvedMostRecentCommonAncestorNode;
class UnresolvedPhyloSubtreeNode;
}  // namespace silo::query_engine::operators

namespace silo::query_engine {

/// Optimization pass that resolves placeholder nodes into concrete, table-backed nodes.
/// - UnresolvedMutationsNode → MutationsNode
/// - UnresolvedInsertionsNode → InsertionsNode
/// - UnresolvedPhyloSubtreeNode → PhyloSubtreeNode
/// - UnresolvedMostRecentCommonAncestorNode → MostRecentCommonAncestorNode
/// - AggregateNode(COUNT(*), TableScanNode) → CountFilterNode
class NodeResolutionPass : public PipelinePassBase<NodeResolutionPass> {
  public:
   static constexpr std::string_view NAME = "NodeResolutionPass";

   using PipelinePassBase<NodeResolutionPass>::operator();

   operators::QueryNodePtr operator()(operators::AggregateNode& node);
   template <typename SymbolType>
   operators::QueryNodePtr operator()(operators::UnresolvedMutationsNode<SymbolType>& node);
   template <typename SymbolType>
   operators::QueryNodePtr operator()(operators::UnresolvedInsertionsNode<SymbolType>& node);
   operators::QueryNodePtr operator()(operators::UnresolvedMostRecentCommonAncestorNode& node);
   operators::QueryNodePtr operator()(operators::UnresolvedPhyloSubtreeNode& node);
};

}  // namespace silo::query_engine
