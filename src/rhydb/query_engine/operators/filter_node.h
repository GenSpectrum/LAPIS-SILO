#pragma once

#include <map>
#include <memory>
#include <vector>

#include <arrow/result.h>

#include "rhydb/query_engine/operators/query_node.h"
#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"
#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/table.h"

namespace rhydb::query_engine::operators {

/// Applies a filter expression to its child's output.
/// Must be eliminated during pushdown before query plan generation.
class FilterNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::unique_ptr<scalar_expressions::ScalarExpression> filter;

   FilterNode(QueryNodePtr child, std::unique_ptr<scalar_expressions::ScalarExpression> filter);

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<arrow::acero::ExecNode*> addToExecPlan(
      arrow::acero::ExecPlan& plan,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;

   [[nodiscard]] NodeKind kind() const override { return NodeKind::FILTER; }

   [[nodiscard]] nlohmann::json toJson() const override;
};

}  // namespace rhydb::query_engine::operators
