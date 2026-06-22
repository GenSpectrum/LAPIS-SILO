#include "silo/query_engine/operators/union_all_node.h"

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <nlohmann/json.hpp>

namespace silo::query_engine::operators {

UnionAllNode::UnionAllNode(QueryNodePtr left, QueryNodePtr right)
    : left(std::move(left)),
      right(std::move(right)) {}

std::vector<schema::ColumnIdentifier> UnionAllNode::getOutputSchema() const {
   return left->getOutputSchema();
}

arrow::Result<arrow::acero::ExecNode*> UnionAllNode::addToExecPlan(
   arrow::acero::ExecPlan& plan,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options
) const {
   ARROW_ASSIGN_OR_RAISE(auto* left_node, left->addToExecPlan(plan, tables, query_options));
   ARROW_ASSIGN_OR_RAISE(auto* right_node, right->addToExecPlan(plan, tables, query_options));
   return arrow::acero::MakeExecNode("union", &plan, {left_node, right_node}, {});
}

nlohmann::json UnionAllNode::toJson() const {
   return {
      {"type", nodeKindToString(kind())},
      {"left", left->toJson()},
      {"right", right->toJson()},
   };
}

}  // namespace silo::query_engine::operators
