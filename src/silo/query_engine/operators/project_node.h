#pragma once

#include <map>
#include <memory>
#include <vector>

#include <arrow/result.h>

#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// Selects specific columns from its child's output.
/// Can be eliminated during pushdown before query plan generation
class ProjectNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::vector<schema::ColumnIdentifier> fields;

   ProjectNode(QueryNodePtr child, std::vector<schema::ColumnIdentifier> fields);

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<arrow::acero::ExecNode*> addToExecPlan(
      arrow::acero::ExecPlan& plan,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;

   [[nodiscard]] NodeKind kind() const override { return NodeKind::PROJECT; }

   [[nodiscard]] nlohmann::json toJson() const override;
};

}  // namespace silo::query_engine::operators
