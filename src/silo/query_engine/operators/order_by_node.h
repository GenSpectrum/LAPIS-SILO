#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string_view>
#include <vector>

#include <arrow/result.h>

#include "silo/query_engine/actions/order_by_field.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// Sorts output rows by the specified fields.
class OrderByNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::vector<OrderByField> fields;
   std::optional<uint32_t> randomize_seed;

   OrderByNode(
      QueryNodePtr child,
      std::vector<OrderByField> fields,
      std::optional<uint32_t> randomize_seed
   );

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;

   [[nodiscard]] std::string_view getType() const override;
};

}  // namespace silo::query_engine::operators
