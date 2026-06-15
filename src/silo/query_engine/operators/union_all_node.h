#pragma once

#include <map>
#include <memory>

#include <arrow/result.h>

#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// Concatenates the output of two child pipelines (UNION ALL semantics).
/// Both children must produce compatible output schemas (same column names and types).
class UnionAllNode final : public QueryNode {
  public:
   QueryNodePtr left;
   QueryNodePtr right;

   UnionAllNode(QueryNodePtr left, QueryNodePtr right);

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;

   [[nodiscard]] NodeKind kind() const override { return NodeKind::UNION_ALL; }

   [[nodiscard]] nlohmann::json toJson() const override;
};

}  // namespace silo::query_engine::operators
