#pragma once

#include <map>
#include <vector>

#include <arrow/result.h>

#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// Leaf node that reports the output schema of whatever it was applied to.
/// Emits one row per described field with two STRING columns: `fieldName` and `type`.
class SchemaNode final : public QueryNode {
  public:
   std::vector<schema::ColumnIdentifier> input_schema;

   explicit SchemaNode(std::vector<schema::ColumnIdentifier> input_schema);

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<arrow::acero::ExecNode*> addToExecPlan(
      arrow::acero::ExecPlan& plan,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;

   [[nodiscard]] NodeKind kind() const override { return NodeKind::SCHEMA; }

   [[nodiscard]] nlohmann::json toJson() const override;
};

}  // namespace silo::query_engine::operators
