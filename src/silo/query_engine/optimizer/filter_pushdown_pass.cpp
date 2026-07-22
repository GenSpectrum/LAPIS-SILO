#include "silo/query_engine/optimizer/filter_pushdown_pass.h"

#include <algorithm>
#include <vector>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/insertions_node.h"
#include "silo/query_engine/operators/join_node.h"
#include "silo/query_engine/operators/most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/mutations_node.h"
#include "silo/query_engine/operators/phylo_subtree_node.h"
#include "silo/query_engine/operators/schema_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/operators/union_all_node.h"
#include "silo/query_engine/scalar_expressions/and.h"

using silo::query_engine::scalar_expressions::And;

namespace silo::query_engine::optimizer {

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::FilterNode& node) {
   current_filters.push_back(std::move(node.filter));
   auto child = std::move(node.child);
   propagateToNode(child);
   return child;
}

operators::QueryNodePtr FilterPushdownPass::operator()(operators::TableScanNode& node) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<And>(std::move(current_filters));
   return nullptr;
}

operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::MutationsNode<silo::Nucleotide>& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<And>(std::move(current_filters));
   return nullptr;
}

operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::MutationsNode<silo::AminoAcid>& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<And>(std::move(current_filters));
   return nullptr;
}
operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::InsertionsNode<silo::Nucleotide>& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<And>(std::move(current_filters));
   return nullptr;
}
operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::InsertionsNode<silo::AminoAcid>& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<And>(std::move(current_filters));
   return nullptr;
}
operators::QueryNodePtr FilterPushdownPass::operator()(operators::PhyloSubtreeNode& node) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<And>(std::move(current_filters));
   return nullptr;
}
operators::QueryNodePtr FilterPushdownPass::operator()(operators::MostRecentCommonAncestorNode& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<And>(std::move(current_filters));
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::SchemaNode& node) {
   // schema() reports a child's output schema; it is a result-producing source with no
   // place to push a predicate into. A filter() applied to its output therefore cannot be
   // realized -> reject the query.
   CHECK_SILO_QUERY(
      current_filters.empty(),
      "filter() cannot be applied to the output of schema(); schema() is a source operator "
      "and its result cannot be filtered. Apply filter() before schema() instead."
   );
   // Push filters down *within* the child subtree using a fresh pass, so that filters inside
   // the child (e.g. `default.filter(...).mutations().schema()`) are pushed into the scan.
   // NodeResolutionPass requires this: it expects a bare table scan beneath
   // mutations()/insertions(). A separate instance is used so the filters from above schema()
   // cannot leak into the child.
   FilterPushdownPass child_pass;
   child_pass.propagateToNode(node.child);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::JoinNode& node) {
   // A predicate sitting above a join is distributed to the input whose output schema
   // provides all of the columns it references: a predicate over left-only columns is
   // evaluated on the left input, a right-only predicate on the right input. A predicate
   // that spans both inputs (or references a column that exists on both sides, making the
   // side ambiguous) cannot be turned into a pre-join filter and is rejected.
   auto left_schema = node.left->getOutputSchema();
   auto right_schema = node.right->getOutputSchema();

   const auto all_names_in = [](const std::vector<schema::ColumnIdentifier>& referenced,
                                const std::vector<schema::ColumnIdentifier>& schema) {
      return std::ranges::all_of(referenced, [&](const auto& column) {
         return std::ranges::any_of(schema, [&](const auto& available) {
            return available.name == column.name;
         });
      });
   };

   FilterPushdownPass left_pass;
   FilterPushdownPass right_pass;
   for (auto& filter : current_filters) {
      auto referenced = filter->freeIUs();
      const bool in_left = all_names_in(referenced, left_schema);
      const bool in_right = all_names_in(referenced, right_schema);
      CHECK_SILO_QUERY(
         !(in_left && in_right && !referenced.empty()),
         "filter() above join() references column(s) that exist on both join inputs; the side is "
         "ambiguous. Apply the filter to one of the join inputs instead."
      );
      if (in_left) {
         left_pass.current_filters.push_back(std::move(filter));
      } else if (in_right) {
         right_pass.current_filters.push_back(std::move(filter));
      } else {
         throw IllegalQueryException(
            "filter() above join() references columns from both join inputs, which cannot be "
            "pushed into a single input. Apply the filter to one of the join inputs instead."
         );
      }
   }
   current_filters.clear();

   left_pass.propagateToNode(node.left);
   right_pass.propagateToNode(node.right);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::UnionAllNode& node) {
   // Push parent filters into both children. Clone for right, move originals into left.
   FilterPushdownPass right_pass;
   for (const auto& filter : current_filters) {
      right_pass.current_filters.push_back(filter->clone());
   }
   FilterPushdownPass left_pass;
   left_pass.current_filters = std::move(current_filters);

   left_pass.propagateToNode(node.left);
   right_pass.propagateToNode(node.right);
   return nullptr;
}

}  // namespace silo::query_engine::optimizer
