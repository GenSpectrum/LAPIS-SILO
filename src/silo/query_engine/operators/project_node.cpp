#include "silo/query_engine/operators/project_node.h"

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/compute/api.h>

namespace silo::query_engine::operators {

ProjectNode::ProjectNode(QueryNodePtr child, std::vector<schema::ColumnIdentifier> fields)
    : child(std::move(child)),
      fields(std::move(fields)) {}

std::vector<schema::ColumnIdentifier> ProjectNode::getOutputSchema() const {
   return fields;
}

arrow::Result<PartialArrowPlan> ProjectNode::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options
) const {
   ARROW_ASSIGN_OR_RAISE(auto plan, child->toQueryPlan(tables, query_options));

   std::vector<arrow::Expression> expressions;
   std::vector<std::string> names;
   expressions.reserve(fields.size());
   names.reserve(fields.size());
   for (const auto& field : fields) {
      expressions.push_back(arrow::compute::field_ref(field.name));
      names.push_back(field.name);
   }

   const arrow::acero::ProjectNodeOptions options{std::move(expressions), std::move(names)};
   ARROW_ASSIGN_OR_RAISE(
      plan.top_node,
      arrow::acero::MakeExecNode("project", plan.plan.get(), {plan.top_node}, options)
   );
   return plan;
}

}  // namespace silo::query_engine::operators
