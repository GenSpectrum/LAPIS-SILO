#include "silo/query_engine/optimizer/column_narrowing_pass.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <vector>

#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/order_by_node.h"
#include "silo/query_engine/operators/project_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/operators/union_all_node.h"

namespace silo::query_engine::optimizer {

ColumnNarrowingPass ColumnNarrowingPass::makePass(const operators::QueryNodePtr& node) {
   return ColumnNarrowingPass{node->getOutputSchema()};
}

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
   propagateToNode(node.child);
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
   propagateToNode(node.child);
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

   const auto already_required = [&](const schema::ColumnIdentifier& column) {
      return std::ranges::any_of(child_required, [&](const auto& existing) {
         return existing.name == column.name;
      });
   };

   // Decide which assignments to keep by index, so that the surviving assignments can be
   // emitted in their original relative order afterwards. Reordering them would change the
   // order in which getOutputSchema appends newly-added columns and thus the output column
   // order of the projection.
   std::vector<bool> keep_assignment(node.assignments.size(), false);

   for (const auto& required_column : required) {
      // for duplicate output names the *last* assignment wins
      auto reversed_assignments_iterator = std::ranges::find_if(
         std::ranges::reverse_view(node.assignments),
         [&](const auto& assignment) {
            return assignment.output_column.name == required_column.name;
         }
      );
      if (reversed_assignments_iterator != node.assignments.rend()) {
         // This assignment produces the required column — keep it and add its inputs.
         for (const auto& referenced_column :
              reversed_assignments_iterator->expression->freeIUs()) {
            if (!already_required(referenced_column)) {
               child_required.push_back(referenced_column);
            }
         }
         // Convert the reverse iterator to a forward iterator: base() points one past the
         // element, so step back one to index the kept assignment.
         const auto index = std::distance(
            node.assignments.begin(), std::next(reversed_assignments_iterator).base()
         );
         keep_assignment[index] = true;
      } else {
         // Pass-through column: the child must provide it directly.
         if (!already_required(required_column)) {
            child_required.push_back(required_column);
         }
      }
   }

   // Keep the surviving assignments in their original relative order.
   std::vector<operators::MapNode::Assignment> kept_assignments;
   for (size_t index = 0; index < node.assignments.size(); ++index) {
      if (keep_assignment[index]) {
         kept_assignments.push_back(std::move(node.assignments[index]));
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
   propagateToNode(node.child);

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
   propagateToNode(node.child);
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion,readability-make-member-function-const)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::UnionAllNode& node) {
   // Narrow columns in each child independently using the same required set. Each branch needs
   // its own pass instance because `required` is mutated while walking, so the base default
   // (which shares a single instance across both branches) is not correct here.
   ColumnNarrowingPass left_pass(required);
   left_pass.propagateToNode(node.left);
   ColumnNarrowingPass right_pass(required);
   right_pass.propagateToNode(node.right);
   return nullptr;
}

}  // namespace silo::query_engine::optimizer
