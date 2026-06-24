#pragma once

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/pipeline_pass_base.h"

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
}  // namespace silo::query_engine::operators

namespace silo::query_engine {

/// Optimization pass that eliminates FilterNodes by pushing their filter expression
/// into the child node's filter field
class FilterPushdownPass : public PipelinePassBase<FilterPushdownPass> {
   std::vector<std::unique_ptr<expressions::Expression>> current_filters;

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

   operators::QueryNodePtr operator()(operators::UnionAllNode& node);
};

}  // namespace silo::query_engine
