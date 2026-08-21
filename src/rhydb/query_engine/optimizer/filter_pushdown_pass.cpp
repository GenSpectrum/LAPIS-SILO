#include "rhydb/query_engine/optimizer/filter_pushdown_pass.h"

#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

#include "rhydb/common/aa_symbols.h"
#include "rhydb/common/nucleotide_symbols.h"
#include "rhydb/query_engine/illegal_query_exception.h"
#include "rhydb/query_engine/operators/filter_node.h"
#include "rhydb/query_engine/operators/insertions_node.h"
#include "rhydb/query_engine/operators/join_node.h"
#include "rhydb/query_engine/operators/map_node.h"
#include "rhydb/query_engine/operators/most_recent_common_ancestor_node.h"
#include "rhydb/query_engine/operators/mutations_node.h"
#include "rhydb/query_engine/operators/phylo_subtree_node.h"
#include "rhydb/query_engine/operators/schema_node.h"
#include "rhydb/query_engine/operators/table_scan_node.h"
#include "rhydb/query_engine/operators/transitive_closure_node.h"
#include "rhydb/query_engine/operators/union_all_node.h"
#include "rhydb/query_engine/scalar_expressions/and.h"
#include "rhydb/query_engine/scalar_expressions/field_ref.h"
#include "rhydb/query_engine/scalar_expressions/zstd_decompress_scalar.h"

using rhydb::query_engine::scalar_expressions::And;

namespace rhydb::query_engine::optimizer {

namespace {

bool isFieldRef(const operators::MapNode::Assignment& assignment) {
   const scalar_expressions::ScalarExpression* expression = assignment.expression.get();
   // Sequence columns cannot be read into an arrow plan directly: they are stored zstd-compressed
   // and decompressed on demand, so as a hack we treat a zstd-decompression as an identity here.
   // A filter referencing such a column is then pushed down and compiles against the physical
   // (compressed) column by name. This may produce misleading error messages in edge cases.
   while (const auto* decompress =
             scalar_expressions::dynCast<scalar_expressions::ZstdDecompressScalar>(expression)) {
      expression = decompress->input.get();
   }
   const auto* field_ref = scalar_expressions::dynCast<scalar_expressions::FieldRef>(expression);
   return field_ref != nullptr && field_ref->column.name == assignment.output_column.name;
}

}  // namespace

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::FilterNode& node) {
   current_filters.push_back(std::move(node.filter));
   auto child = std::move(node.child);
   propagateToNode(child);
   return child;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::MapNode& node) {
   // The columns this map produces that a filter may NOT be pushed past, keyed by name. A
   // produced column blocks pushdown unless its assignment just reproduces the same-named column,
   // which the scan can still evaluate by name. A filter referencing a blocking
   // column reads a value that does not exist physically below the map, so pushing it down would
   // turn a valid plan into an invalid one (see #1371).
   //
   // Matching is by NAME: the produced column and a referencing filter carry identical
   // {name, type} identifiers (e.g. a decompression map's {segment1, STRING} output and a
   // hasMutation predicate resolved against it), so only the assignment's expression can
   // distinguish an unchanged column from a genuinely derived value.
   //
   // TODO(#1433): instead of blocking, fold the producing map's expression into the filter
   std::unordered_set<std::string> blocking_columns;
   for (const auto& assignment : node.assignments) {
      if (!isFieldRef(assignment)) {
         blocking_columns.insert(assignment.output_column.name);
      }
   }

   // Partition the accumulated filters: those referencing a blocking column must stay above
   // this map; the rest can continue to be pushed further down into the child.
   std::vector<std::unique_ptr<scalar_expressions::ScalarExpression>> filters_staying_above;
   std::vector<std::unique_ptr<scalar_expressions::ScalarExpression>> filters_to_push_down;
   for (auto& filter : current_filters) {
      const auto references_blocking_column =
         std::ranges::any_of(filter->freeIUs(), [&](const auto& column) {
            return blocking_columns.contains(column.name);
         });
      if (references_blocking_column) {
         filters_staying_above.push_back(std::move(filter));
      } else {
         filters_to_push_down.push_back(std::move(filter));
      }
   }

   current_filters = std::move(filters_to_push_down);
   propagateToNode(node.child);

   if (filters_staying_above.empty()) {
      // Every filter could be pushed below the map; keep the map in place.
      return nullptr;
   }

   // Rebuild the map (its child may have been rewritten by the pushdown above) and place the
   // remaining filters back on top of it as a single FilterNode. It cannot be pushed further.
   auto rebuilt_map =
      std::make_unique<operators::MapNode>(std::move(node.child), std::move(node.assignments));
   auto remaining_filter = std::make_unique<And>(std::move(filters_staying_above));
   return std::make_unique<operators::FilterNode>(
      std::move(rebuilt_map), std::move(remaining_filter)
   );
}

operators::QueryNodePtr FilterPushdownPass::operator()(operators::TableScanNode& node) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<And>(std::move(current_filters));
   return nullptr;
}

operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::MutationsNode<rhydb::Nucleotide>& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<And>(std::move(current_filters));
   return nullptr;
}

operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::MutationsNode<rhydb::AminoAcid>& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<And>(std::move(current_filters));
   return nullptr;
}
operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::InsertionsNode<rhydb::Nucleotide>& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<And>(std::move(current_filters));
   return nullptr;
}
operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::InsertionsNode<rhydb::AminoAcid>& node
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
operators::QueryNodePtr FilterPushdownPass::operator()(operators::TransitiveClosureNode& node) {
   // transitiveClosure() re-materializes its child into a fresh from/to relation; it is a
   // source operator with no place to push a predicate into. A filter() applied to its output
   // therefore cannot be realized -> reject the query.
   CHECK_SILO_QUERY(
      current_filters.empty(),
      "filter() cannot be applied to the output of transitiveClosure(); transitiveClosure() is "
      "a source operator and its result cannot be filtered. Apply filter() to its input instead."
   );
   // Push filters down within the child subtree using a fresh pass so nothing leaks across the
   // materialization boundary (e.g. `relation.filter(...).transitiveClosure(...)`).
   FilterPushdownPass child_pass;
   child_pass.propagateToNode(node.child);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::JoinNode& node) {
   // A filter sitting above a join is not turned into a pre-join filter. Which input a
   // predicate belongs to could be derived from freeIUs(), but pushing is only
   // semantics-preserving for some combinations: pushing into the null-supplying side of an
   // outer join changes the result (null-extended rows would no longer be filtered out), as
   // does pushing a predicate that references no column at all. Rather than push unsafely,
   // reject any filter above join() and point the user at the inputs.
   CHECK_SILO_QUERY(
      current_filters.empty(),
      "filter() cannot be applied to the output of join(); a filter above a join cannot be "
      "pushed into a join input safely. Apply the filter to one of the join inputs instead."
   );

   // No filters to carry across, but the child subtrees may still contain FilterNodes of
   // their own (e.g. `join(default.filter(...), ...)`); push those down within each input
   // using fresh passes so no state leaks between the two branches.
   FilterPushdownPass left_pass;
   FilterPushdownPass right_pass;
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

}  // namespace rhydb::query_engine::optimizer
