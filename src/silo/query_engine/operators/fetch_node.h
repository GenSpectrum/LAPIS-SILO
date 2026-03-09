#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string_view>
#include <vector>

#include <arrow/result.h>

#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

class FetchNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::optional<uint32_t> count;
   std::optional<uint32_t> offset;

   FetchNode(QueryNodePtr child, std::optional<uint32_t> count, std::optional<uint32_t> offset);

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;
};

}  // namespace silo::query_engine::operators
