#include "silo/query_engine/column_narrowing_pass.h"

#include <algorithm>
#include <iterator>
#include <ranges>

#include "silo/query_engine/operator_visitor.h"
#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/order_by_node.h"
#include "silo/query_engine/operators/project_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/operators/union_all_node.h"
#include "silo/query_engine/operators/unresolved_insertions_node.h"
#include "silo/query_engine/operators/unresolved_most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/unresolved_mutations_node.h"
#include "silo/query_engine/operators/unresolved_phylo_subtree_node.h"

namespace silo::query_engine {

namespace {
// NOLINTNEXTLINE(misc-no-recursion)
void applyToChild(operators::QueryNodePtr& child, ColumnNarrowingPass& pass) {
   if (auto new_child = operators::visit(*child, pass)) {
      child = std::move(new_child);
   }
}
}  // namespace

// NOLINTNEXTLINE(readability-convert-member-functions-to-static,misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::TableScanNode& node) {
   std::vector<schema::ColumnIdentifier> pruned;
   for (const auto& col : node.fields) {
      if (std::ranges::find(required, col) != required.end()) {
         pruned.push_back(col);
      }
   }
   node.fields = std::move(pruned);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::AggregateNode& node) {
   std::vector<schema::ColumnIdentifier> child_required;
   child_required.reserve(node.group_by_fields.size());
   for (const auto& field : node.group_by_fields) {
      child_required.push_back(field);
   }
   for (const auto& agg : node.aggregates) {
      if (agg.source_column.has_value()) {
         child_required.push_back(agg.source_column.value());
      }
   }

   if (child_required.empty()) {
      // COUNT(*) with no group-by: still need one column to drive the row stream.
      auto child_schema = node.child->getOutputSchema();
      SILO_ASSERT(!child_schema.empty());
      required = RequiredColumns{child_schema.front()};
   } else {
      required = std::move(child_required);
   }
   applyToChild(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::ProjectNode& node) {
   std::vector<schema::ColumnIdentifier> narrowed_fields;
   for (const auto& field : node.fields) {
      if (std::ranges::find(required, field) != required.end()) {
         narrowed_fields.push_back(field);
      }
   }
   node.fields = narrowed_fields;

   required = node.fields;
   applyToChild(node.child, *this);
   if (node.child->getOutputSchema() == node.fields) {
      return std::move(node.child);
   }
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::MapNode& node) {
   // A MapNode keeps all of its child's columns and adds (or replaces) some via
   // scalar assignments. We only keep assignments that produce required output columns;
   // the child must provide the referenced columns for those assignments plus any
   // required pass-through columns.
   //
   // Output columns are identified by name (MapNode::getOutputSchema and addToExecPlan
   // both key assignments by name), so we match a required column to an assignment by
   // name. The child likewise cannot expose two columns of the same name, so child
   // requirements are deduplicated by name rather than by full ColumnIdentifier — a
   // decompression assignment requests its input column as a sequence type while a
   // pass-through of the same name would request it as STRING.
   RequiredColumns child_required;
   std::vector<operators::MapNode::Assignment> kept_assignments;

   const auto already_required = [&](const schema::ColumnIdentifier& column) {
      return std::ranges::any_of(child_required, [&](const auto& existing) {
         return existing.name == column.name;
      });
   };

   for (const auto& required_column : required) {
      // getOutputSchema/addToExecPlan apply assignments front-to-back, so for duplicate
      // output names the *last* assignment wins. Match the last one to preserve that
      // behavior; earlier duplicates are dead and dropped together with their inputs.
      auto reverse_it = std::ranges::find_if(
         std::ranges::reverse_view(node.assignments),
         [&](const auto& assignment) {
            return assignment.output_column.name == required_column.name;
         }
      );
      if (reverse_it != node.assignments.rend()) {
         // This assignment produces the required column — keep it and add its inputs.
         for (const auto& referenced_column : reverse_it->expression->freeIUs()) {
            if (!already_required(referenced_column)) {
               child_required.push_back(referenced_column);
            }
         }
         // Convert the reverse iterator to a forward iterator for erase: base() points one
         // past the element, so step back one.
         auto it = std::next(reverse_it).base();
         kept_assignments.push_back(std::move(*it));
         node.assignments.erase(it);
      } else {
         // Pass-through column: the child must provide it directly.
         if (!already_required(required_column)) {
            child_required.push_back(required_column);
         }
      }
   }
   node.assignments = std::move(kept_assignments);

   if (child_required.empty()) {
      // Even when all output columns are produced by assignments, the child must
      // emit at least one field so that row identity is preserved.
      auto child_schema = node.child->getOutputSchema();
      SILO_ASSERT(!child_schema.empty());
      child_required.push_back(child_schema.front());
   }
   required = std::move(child_required);
   applyToChild(node.child, *this);

   if (node.assignments.empty()) {
      return std::move(node.child);
   }
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::OrderByNode& node) {
   for (const auto& field : node.fields) {
      if (std::ranges::find(required, field.field) == required.end()) {
         required.push_back(field.field);
      }
   }
   applyToChild(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::FetchNode& node) {
   applyToChild(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::FilterNode& node) {
   applyToChild(node.child, *this);
   return nullptr;
}

template <typename SymbolType>
// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(
   operators::UnresolvedMutationsNode<SymbolType>& node
) {
   applyToChild(node.child, *this);
   return nullptr;
}

template <typename SymbolType>
// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(
   operators::UnresolvedInsertionsNode<SymbolType>& node
) {
   applyToChild(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(
   operators::UnresolvedMostRecentCommonAncestorNode& node
) {
   applyToChild(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::UnresolvedPhyloSubtreeNode& node
) {
   applyToChild(node.child, *this);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion,readability-make-member-function-const)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::UnionAllNode& node) {
   // Narrow columns in each child independently using the same required set.
   ColumnNarrowingPass left_pass(required);
   applyToChild(node.left, left_pass);
   ColumnNarrowingPass right_pass(required);
   applyToChild(node.right, right_pass);
   return nullptr;
}

template operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::UnresolvedMutationsNode<
                                                                 silo::Nucleotide>&);
template operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::UnresolvedMutationsNode<
                                                                 silo::AminoAcid>&);
template operators::QueryNodePtr ColumnNarrowingPass::
operator()(operators::UnresolvedInsertionsNode<silo::Nucleotide>&);
template operators::QueryNodePtr ColumnNarrowingPass::
operator()(operators::UnresolvedInsertionsNode<silo::AminoAcid>&);

}  // namespace silo::query_engine
