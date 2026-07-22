#pragma once

#include <map>
#include <memory>
#include <vector>

#include <arrow/acero/options.h>
#include <arrow/result.h>
#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// Joins two child pipelines on an equality condition (equi-join), backed by
/// Arrow's hash-join. `left_keys[i]` is compared for equality against
/// `right_keys[i]`; all key pairs must match for a row pair to join.
///
/// The output schema is the left child's columns followed by the right child's
/// columns, except for semi/anti joins which output only the columns of the
/// side they keep. Overlapping column names between the two sides are passed
/// through as-is (mirroring SQL `SELECT *`); rename or project them downstream
/// if distinct names are required.
class JoinNode final : public QueryNode {
  public:
   QueryNodePtr left;
   QueryNodePtr right;
   std::vector<schema::ColumnIdentifier> left_keys;
   std::vector<schema::ColumnIdentifier> right_keys;
   arrow::acero::JoinType join_type;

   JoinNode(
      QueryNodePtr left,
      QueryNodePtr right,
      std::vector<schema::ColumnIdentifier> left_keys,
      std::vector<schema::ColumnIdentifier> right_keys,
      arrow::acero::JoinType join_type
   );

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<arrow::acero::ExecNode*> addToExecPlan(
      arrow::acero::ExecPlan& plan,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;

   [[nodiscard]] NodeKind kind() const override { return NodeKind::JOIN; }

   [[nodiscard]] nlohmann::json toJson() const override;
};

/// Human-readable name for a join type, used in query-plan JSON and diagnostics.
[[nodiscard]] std::string_view joinTypeToString(arrow::acero::JoinType join_type);

}  // namespace silo::query_engine::operators
