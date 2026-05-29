#pragma once

#include <vector>

#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::operators {
class TableScanNode;
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
}  // namespace silo::query_engine::operators

namespace silo::query_engine {

class ColumnNarrowingPass {
  public:
   using RequiredColumns = std::vector<schema::ColumnIdentifier>;

   RequiredColumns required;

   explicit ColumnNarrowingPass(RequiredColumns required)
       : required(std::move(required)) {}

   operators::QueryNodePtr operator()(operators::TableScanNode& node);
   operators::QueryNodePtr operator()(operators::AggregateNode& node);
   operators::QueryNodePtr operator()(operators::ProjectNode& node);
   operators::QueryNodePtr operator()(operators::OrderByNode& node);
   operators::QueryNodePtr operator()(operators::FetchNode& node);
   operators::QueryNodePtr operator()(operators::FilterNode& node);
   template <typename SymbolType>
   operators::QueryNodePtr operator()(operators::UnresolvedMutationsNode<SymbolType>& node);
   template <typename SymbolType>
   operators::QueryNodePtr operator()(operators::UnresolvedInsertionsNode<SymbolType>& node);
   operators::QueryNodePtr operator()(operators::UnresolvedMostRecentCommonAncestorNode& node);
   operators::QueryNodePtr operator()(operators::UnresolvedPhyloSubtreeNode& node);

   // Post-pushdown nodes are leaves that don't participate in column narrowing.
   template <typename T>
   operators::QueryNodePtr operator()(T& /*node*/) {
      return nullptr;
   }
};

}  // namespace silo::query_engine
