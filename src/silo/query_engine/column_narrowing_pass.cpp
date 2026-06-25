#include "silo/query_engine/column_narrowing_pass.h"

#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/order_by_node.h"
#include "silo/query_engine/operators/project_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/operators/union_all_node.h"
#include "silo/query_engine/operators/zstd_decompress_node.h"

namespace silo::query_engine {

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
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::ZstdDecompressNode& node) {
   RequiredColumns child_required;
   std::vector<operators::ZstdDecompressNode::ColumnMapping> new_column_mapping;
   for (const auto& req : required) {
      auto it = std::ranges::find_if(node.column_mapping, [&](const auto& mapping) {
         return mapping.output == req;
      });
      if (it != node.column_mapping.end()) {
         child_required.push_back(it->input);
         new_column_mapping.push_back(std::move(*it));
         node.column_mapping.erase(it);
      } else {
         child_required.push_back(req);
      }
   }
   required = std::move(child_required);
   propagateToNode(node.child);

   if (new_column_mapping.empty()) {
      return std::move(node.child);
   }
   node.column_mapping = new_column_mapping;
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr ColumnNarrowingPass::operator()(operators::MapNode& node) {
   // A map keeps all of its child's columns and adds (or replaces) some via
   // scalar assignments. The child must still provide the required pass-through
   // columns, plus any column referenced by an assignment (e.g. `y := age`):
   // the map projects every assignment, so a referenced column is needed even if
   // the assigned output column is not required downstream.
   RequiredColumns child_required;
   for (const auto& required_column : required) {
      const bool produced_by_map =
         std::ranges::any_of(node.assignments, [&](const auto& assignment) {
            return assignment.output_column.name == required_column.name;
         });
      if (!produced_by_map) {
         child_required.push_back(required_column);
      }
   }
   for (const auto& assignment : node.assignments) {
      for (auto& referenced_column : assignment.expression->freeIUs()) {
         if (std::ranges::find(child_required, referenced_column) == child_required.end()) {
            child_required.push_back(std::move(referenced_column));
         }
      }
   }
   if (child_required.empty()) {
      auto child_schema = node.child->getOutputSchema();
      SILO_ASSERT(!child_schema.empty());
      child_required.push_back(child_schema.front());
   }
   required = std::move(child_required);
   propagateToNode(node.child);
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

}  // namespace silo::query_engine
