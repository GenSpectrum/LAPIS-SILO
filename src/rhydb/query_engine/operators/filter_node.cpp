#include "rhydb/query_engine/operators/filter_node.h"

#include <utility>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/compute/expression.h>
#include <nlohmann/json.hpp>

namespace rhydb::query_engine::operators {

FilterNode::FilterNode(
   QueryNodePtr child,
   std::unique_ptr<scalar_expressions::ScalarExpression> filter
)
    : child(std::move(child)),
      filter(std::move(filter)) {}

std::vector<schema::ColumnIdentifier> FilterNode::getOutputSchema() const {
   return child->getOutputSchema();
}

arrow::Result<arrow::acero::ExecNode*> FilterNode::addToExecPlan(
   arrow::acero::ExecPlan& plan,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options
) const {
   ARROW_ASSIGN_OR_RAISE(auto* child_node, child->addToExecPlan(plan, tables, query_options));
   ARROW_ASSIGN_OR_RAISE(auto filter_expression, filter->toArrowExpression());
   const arrow::acero::FilterNodeOptions options{std::move(filter_expression)};
   return arrow::acero::MakeExecNode("filter", &plan, {child_node}, options);
}

nlohmann::json FilterNode::toJson() const {
   return {
      {"type", nodeKindToString(kind())},
      {"filter", filter->toString()},
      {"child", child->toJson()},
   };
}

}  // namespace rhydb::query_engine::operators
