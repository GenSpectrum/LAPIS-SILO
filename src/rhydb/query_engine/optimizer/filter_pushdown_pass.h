#pragma once

#include "rhydb/query_engine/operators/query_node.h"
#include "rhydb/query_engine/optimizer/pipeline_pass_base.h"
#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"

namespace rhydb::query_engine::operators {
class FilterNode;
class MapNode;
class ProjectNode;
class OrderByNode;
class TableScanNode;
class UnionAllNode;
class JoinNode;
class SchemaNode;
class FetchNode;
class OrderByWithLimitNode;
class AggregateNode;
template <typename SymbolType>
class UnresolvedMutationsNode;
template <typename SymbolType>
class UnresolvedInsertionsNode;
class UnresolvedPhyloSubtreeNode;
class UnresolvedMostRecentCommonAncestorNode;
}  // namespace rhydb::query_engine::operators

namespace rhydb::query_engine::optimizer {

/// Pushes filters as deep into the plan as is semantics-preserving.
///
/// Fail-closed: a node breaks pushdown unless it opts in. `propagateToNode` retains any filter a
/// node did not push down or consume as a FilterNode above it, so an unclassified (or future) node
/// can never silently let a filter through.
class FilterPushdownPass : public PipelinePassBase<FilterPushdownPass> {
   std::vector<std::unique_ptr<scalar_expressions::ScalarExpression>> current_filters;

   /// Adds a filter to `current_filters`, flattening a top-level `And` into its conjuncts so each
   /// can be pushed independently.
   void addFilter(std::unique_ptr<scalar_expressions::ScalarExpression> filter);

  public:
   /// Visits `node`, then wraps any filters it left pending into a FilterNode above it. This is what
   /// makes "break pushdown" the default.
   void propagateToNode(operators::QueryNodePtr& node);

   // Transparent: push filters down into the child (row set and referenced columns unchanged).
   operators::QueryNodePtr operator()(operators::ProjectNode& node);
   operators::QueryNodePtr operator()(operators::OrderByNode& node);

   // Consumes the filters into the scan's own filter field (the bitmap pre-filter).
   operators::QueryNodePtr operator()(operators::TableScanNode& node);

   // Bespoke handling.
   operators::QueryNodePtr operator()(operators::FilterNode& node);
   operators::QueryNodePtr operator()(operators::MapNode& node);
   operators::QueryNodePtr operator()(operators::UnionAllNode& node);
   operators::QueryNodePtr operator()(operators::SchemaNode& node);

   // Barriers: push filters inside their own child subtree down with a fresh pass; filters from
   // above are retained by `propagateToNode`.
   operators::QueryNodePtr operator()(operators::JoinNode& node);
   operators::QueryNodePtr operator()(operators::FetchNode& node);
   operators::QueryNodePtr operator()(operators::OrderByWithLimitNode& node);
   operators::QueryNodePtr operator()(operators::AggregateNode& node);
   template <typename SymbolType>
   operators::QueryNodePtr operator()(operators::UnresolvedMutationsNode<SymbolType>& node);
   template <typename SymbolType>
   operators::QueryNodePtr operator()(operators::UnresolvedInsertionsNode<SymbolType>& node);
   operators::QueryNodePtr operator()(operators::UnresolvedPhyloSubtreeNode& node);
   operators::QueryNodePtr operator()(operators::UnresolvedMostRecentCommonAncestorNode& node);

   // Fail-closed default: any other node is a barrier. Leaf/source nodes reach here with
   // no pending filters and are left untouched.
   template <typename T>
   operators::QueryNodePtr operator()(T& /*node*/) {
      return nullptr;
   }
};

}  // namespace rhydb::query_engine::optimizer
