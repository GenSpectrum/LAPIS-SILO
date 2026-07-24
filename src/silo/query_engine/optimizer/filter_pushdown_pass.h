#pragma once

#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/optimizer/pipeline_pass_base.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"

namespace silo::query_engine::operators {
class FilterNode;
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
}  // namespace silo::query_engine::operators

namespace silo::query_engine::optimizer {

/// Optimization pass that eliminates FilterNodes by pushing their filter expression
/// into the child node's filter field
class FilterPushdownPass : public PipelinePassBase<FilterPushdownPass> {
   std::vector<std::unique_ptr<scalar_expressions::ScalarExpression>> current_filters;

  public:
   using PipelinePassBase<FilterPushdownPass>::operator();

   operators::QueryNodePtr operator()(operators::FilterNode& node);

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

}  // namespace silo::query_engine::optimizer
