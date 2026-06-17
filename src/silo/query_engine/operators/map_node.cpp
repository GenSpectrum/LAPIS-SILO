#include "silo/query_engine/operators/map_node.h"

#include <algorithm>
#include <string>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/compute/api.h>
#include <arrow/datum.h>
#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/expressions/at.h"
#include "silo/query_engine/expressions/field_ref.h"
#include "silo/query_engine/expressions/literal.h"

namespace silo::query_engine::operators {

namespace {

/// Translates a scalar expression into an Arrow compute expression for use in a
/// projection. Literals and column references are supported; a boolean filter
/// predicate would need to be evaluated against the column store first.
arrow::Result<arrow::compute::Expression> scalarToArrowExpression(
   const expressions::Expression& expression
) {
   if (const auto* literal = dynamic_cast<const expressions::Int64Literal*>(&expression)) {
      return arrow::compute::literal(arrow::Datum(literal->value));
   }
   if (const auto* literal = dynamic_cast<const expressions::FloatLiteral*>(&expression)) {
      return arrow::compute::literal(arrow::Datum(literal->value));
   }
   if (const auto* literal = dynamic_cast<const expressions::StringLiteral*>(&expression)) {
      return arrow::compute::literal(arrow::Datum(literal->value));
   }
   if (const auto* literal = dynamic_cast<const expressions::BoolLiteral*>(&expression)) {
      return arrow::compute::literal(arrow::Datum(literal->value));
   }
   if (const auto* field_ref = dynamic_cast<const expressions::FieldRef*>(&expression)) {
      return arrow::compute::field_ref(field_ref->column.name);
   }
   if (const auto* at_function = dynamic_cast<const expressions::At*>(&expression)) {
      // `at` is 1-indexed; utf8_slice_codeunits takes a 0-indexed, half-open
      // [start, stop) range of code units, so extract the single character at
      // position-1.
      const int64_t start = static_cast<int64_t>(at_function->position) - 1;
      const auto stop = static_cast<int64_t>(at_function->position);
      return arrow::compute::call(
         "utf8_slice_codeunits",
         {arrow::compute::field_ref(at_function->input_column.name)},
         arrow::compute::SliceOptions(start, stop)
      );
   }
   return arrow::Status::NotImplemented(
      "unsupported scalar expression in map() assignments (supported: literals, column references, "
      "and at())"
   );
}

}  // namespace

MapNode::MapNode(QueryNodePtr child, std::vector<Assignment> assignments)
    : child(std::move(child)),
      assignments(std::move(assignments)) {}

std::vector<schema::ColumnIdentifier> MapNode::getOutputSchema() const {
   auto output = child->getOutputSchema();
   for (const auto& assignment : assignments) {
      auto found = std::ranges::find_if(output, [&](const auto& column) {
         return column.name == assignment.output_column.name;
      });
      if (found != output.end()) {
         *found = assignment.output_column;
      } else {
         output.push_back(assignment.output_column);
      }
   }
   return output;
}

arrow::Result<arrow::acero::ExecNode*> MapNode::addToExecPlan(
   arrow::acero::ExecPlan& plan,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options
) const {
   ARROW_ASSIGN_OR_RAISE(auto* child_node, child->addToExecPlan(plan, tables, query_options));

   // Map output column names to the assignment that produces them.
   std::map<std::string, const Assignment*> assignment_by_name;
   for (const auto& assignment : assignments) {
      assignment_by_name[assignment.output_column.name] = &assignment;
   }

   const auto output_schema = getOutputSchema();
   std::vector<arrow::Expression> expressions;
   std::vector<std::string> names;
   expressions.reserve(output_schema.size());
   names.reserve(output_schema.size());
   for (const auto& [name, type] : output_schema) {
      names.push_back(name);
      auto found = assignment_by_name.find(name);
      if (found == assignment_by_name.end()) {
         // Column passed through unchanged from the child.
         expressions.push_back(arrow::compute::field_ref(name));
         continue;
      }
      ARROW_ASSIGN_OR_RAISE(
         auto arrow_expression, scalarToArrowExpression(*found->second->expression)
      );
      expressions.push_back(std::move(arrow_expression));
   }

   const arrow::acero::ProjectNodeOptions options{std::move(expressions), std::move(names)};
   return arrow::acero::MakeExecNode("project", &plan, {child_node}, options);
}

nlohmann::json MapNode::toJson() const {
   nlohmann::json map_expressions = nlohmann::json::array();
   for (const auto& [field, expression] : assignments) {
      map_expressions.push_back(nlohmann::json{field.name, expression->toString()});
   }
   return nlohmann::json{
      {"type", nodeKindToString(kind())},
      {"child", child->toJson()},
      {"mapExpressions", map_expressions}
   };
}

}  // namespace silo::query_engine::operators
