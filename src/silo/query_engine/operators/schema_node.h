#pragma once

#include <map>
#include <string_view>
#include <vector>

#include <arrow/result.h>

#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// Terminal node that reports the output schema of its child as data rows.
/// Emits one row per child field.
/// The child's query plan is never executed; only its output schema is inspected.
class SchemaNode final : public QueryNode {
  public:
   static constexpr std::string_view FIELD_NAME_COLUMN = "fieldName";
   static constexpr std::string_view TYPE_COLUMN = "type";

   QueryNodePtr child;

   explicit SchemaNode(QueryNodePtr child);

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
