#pragma once

#include "rhydb/query_engine/operators/query_node.h"
#include "rhydb/query_engine/optimizer/pipeline_pass_base.h"
#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"

namespace rhydb::query_engine::operators {
class FilterNode;
class MapNode;
class TableScanNode;
template <typename SymbolType>
class MutationsNode;
template <typename SymbolType>
class InsertionsNode;
class PhyloSubtreeNode;
class MostRecentCommonAncestorNode;
class UnionAllNode;
class JoinNode;
class SchemaNode;
}  // namespace rhydb::query_engine::operators

namespace rhydb::query_engine::optimizer {

/// Optimization pass that eliminates FilterNodes by pushing their filter expression
/// into the child node's filter field
class FilterPushdownPass : public PipelinePassBase<FilterPushdownPass> {
   std::vector<std::unique_ptr<scalar_expressions::ScalarExpression>> current_filters;

   /// Adds a filter to `current_filters`, splitting a top-level conjunction (`And`) into its
   /// individual conjuncts so each can be pushed to the deepest node that supports it.
   void addFilter(std::unique_ptr<scalar_expressions::ScalarExpression> filter);

  public:
   using PipelinePassBase<FilterPushdownPass>::operator();

   operators::QueryNodePtr operator()(operators::FilterNode& node);
   operators::QueryNodePtr operator()(operators::MapNode& node);

   operators::QueryNodePtr operator()(operators::TableScanNode& node);
   operators::QueryNodePtr operator()(operators::MutationsNode<Nucleotide>& node);
   operators::QueryNodePtr operator()(operators::MutationsNode<AminoAcid>& node);
   operators::QueryNodePtr operator()(operators::InsertionsNode<Nucleotide>& node);
   operators::QueryNodePtr operator()(operators::InsertionsNode<AminoAcid>& node);
   operators::QueryNodePtr operator()(operators::PhyloSubtreeNode& node);
   operators::QueryNodePtr operator()(operators::MostRecentCommonAncestorNode& node);
   operators::QueryNodePtr operator()(operators::SchemaNode& node);

   operators::QueryNodePtr operator()(operators::UnionAllNode& node);

   operators::QueryNodePtr operator()(operators::JoinNode& node);
};

}  // namespace rhydb::query_engine::optimizer
