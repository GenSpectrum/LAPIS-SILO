#pragma once

#include <map>
#include <memory>

#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {
class AggregateNode;
class ProjectNode;
class OrderByNode;
class FetchNode;
class FilterNode;
template <typename SymbolType>
class UnresolvedMutationsNode;
template <typename SymbolType>
class UnresolvedInsertionsNode;
class UnresolvedMostRecentCommonAncestorNode;
class UnresolvedPhyloSubtreeNode;
class ZstdDecompressNode;
}  // namespace silo::query_engine::operators

namespace silo::query_engine {

/// Optimization pass that converts abstract placeholder nodes into concrete, table-backed nodes.
/// - ScanNode → TableScanNode (with True filter)
/// - FilterNode(ScanNode) → TableScanNode (with filter embedded, FilterNode eliminated)
/// - UnresolvedMutationsNode → MutationsNode
/// - UnresolvedInsertionsNode → InsertionsNode
/// - UnresolvedPhyloSubtreeNode → PhyloSubtreeNode
/// - UnresolvedMostRecentCommonAncestorNode → MostRecentCommonAncestorNode
class NodeResolutionPass {
  public:
   using Tables = std::map<schema::TableName, std::shared_ptr<storage::Table>>;

   static operators::QueryNodePtr run(operators::QueryNodePtr node);

   operators::QueryNodePtr operator()(operators::FilterNode& node);
   operators::QueryNodePtr operator()(operators::AggregateNode& node);
   operators::QueryNodePtr operator()(operators::ProjectNode& node);
   operators::QueryNodePtr operator()(operators::OrderByNode& node);
   operators::QueryNodePtr operator()(operators::FetchNode& node);
   operators::QueryNodePtr operator()(operators::ZstdDecompressNode& node);
   template <typename SymbolType>
   operators::QueryNodePtr operator()(operators::UnresolvedMutationsNode<SymbolType>& node);
   template <typename SymbolType>
   operators::QueryNodePtr operator()(operators::UnresolvedInsertionsNode<SymbolType>& node);
   operators::QueryNodePtr operator()(operators::UnresolvedMostRecentCommonAncestorNode& node);
   operators::QueryNodePtr operator()(operators::UnresolvedPhyloSubtreeNode& node);

   // Post-resolution nodes are leaves that don't participate in this pass.
   template <typename T>
   operators::QueryNodePtr operator()(T& /*node*/) {
      return nullptr;
   }
};

}  // namespace silo::query_engine
