#pragma once

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/pipeline_pass_base.h"

namespace silo::query_engine::operators {
class AggregateNode;
class FilterNode;
class TableScanNode;
template <typename SymbolType>
class MutationsNode;
template <typename SymbolType>
class InsertionsNode;
class PhyloSubtreeNode;
class MostRecentCommonAncestorNode;
template <typename SymbolType>
class UnresolvedMutationsNode;
template <typename SymbolType>
class UnresolvedInsertionsNode;
class UnresolvedPhyloSubtreeNode;
class UnresolvedMostRecentCommonAncestorNode;
class UnionAllNode;

}  // namespace silo::query_engine::operators

namespace silo::query_engine {

/// Optimization pass that eliminates FilterNodes by pushing their filter expression
/// into the child node's filter field
class FilterPushdownPass : public PipelinePassBase<FilterPushdownPass> {
   std::vector<std::unique_ptr<expressions::Expression>> current_filters;

  public:
   using PipelinePassBase<FilterPushdownPass>::operator();

   static operators::QueryNodePtr run(operators::QueryNodePtr node);

   operators::QueryNodePtr operator()(operators::FilterNode& node);
   operators::QueryNodePtr operator()(operators::AggregateNode& node);

   operators::QueryNodePtr operator()(operators::TableScanNode& node);
   operators::QueryNodePtr operator()(operators::MutationsNode<Nucleotide>& node);
   operators::QueryNodePtr operator()(operators::MutationsNode<AminoAcid>& node);
   operators::QueryNodePtr operator()(operators::InsertionsNode<Nucleotide>& node);
   operators::QueryNodePtr operator()(operators::InsertionsNode<AminoAcid>& node);
   operators::QueryNodePtr operator()(operators::PhyloSubtreeNode& node);
   operators::QueryNodePtr operator()(operators::MostRecentCommonAncestorNode& node);

   template <typename SymbolType>
   operators::QueryNodePtr operator()(operators::UnresolvedMutationsNode<SymbolType>& node);
   template <typename SymbolType>
   operators::QueryNodePtr operator()(operators::UnresolvedInsertionsNode<SymbolType>& node);
   operators::QueryNodePtr operator()(operators::UnresolvedPhyloSubtreeNode& node);
   operators::QueryNodePtr operator()(operators::UnresolvedMostRecentCommonAncestorNode& node);
   operators::QueryNodePtr operator()(operators::UnionAllNode& node);

   // All other nodes are leaves here.
   template <typename T>
   operators::QueryNodePtr operator()(T& /*node*/) {
      return nullptr;
   }
};

}  // namespace silo::query_engine
