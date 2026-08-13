#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include <arrow/result.h>

#include "rhydb/query_engine/operators/query_node.h"
#include "rhydb/query_engine/order_by_field.h"
#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/table.h"

namespace rhydb::query_engine::operators {

/// Sorts output rows by the specified fields and keeps only the `offset + limit` smallest of them.
///
/// This is the combined equivalent of an `OrderByNode` directly below a `FetchNode`: instead of
/// sorting the entire input and then discarding all but a small window, it uses Arrow's `select_k`
/// (top-k) implementation, which only ever keeps the `offset + limit` best rows seen so far. The
/// `SelectKRewritePass` rewrites `Fetch(OrderBy(...))` into this node.
///
/// Like `OrderByNode`, a `randomize_seed` orders rows by a per-row random hash (as an extra,
/// lowest-priority sort key when `fields` are also present), so a randomized query followed by a
/// limit selects a random sample via the same top-k path.
class OrderByWithLimitNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::vector<OrderByField> fields;
   uint32_t limit;
   std::optional<uint32_t> offset;
   std::optional<uint32_t> randomize_seed;

   OrderByWithLimitNode(
      QueryNodePtr child,
      std::vector<OrderByField> fields,
      uint32_t limit,
      std::optional<uint32_t> offset,
      std::optional<uint32_t> randomize_seed
   );

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<arrow::acero::ExecNode*> addToExecPlan(
      arrow::acero::ExecPlan& plan,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;

   [[nodiscard]] NodeKind kind() const override { return NodeKind::ORDER_BY_WITH_LIMIT; }

   [[nodiscard]] nlohmann::json toJson() const override;
};

}  // namespace rhydb::query_engine::operators
