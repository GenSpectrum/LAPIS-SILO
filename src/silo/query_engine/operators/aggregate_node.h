#pragma once

#include <map>
#include <string_view>
#include <vector>

#include <arrow/result.h>

#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// Counts rows, optionally grouped by fields.
class AggregateNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::vector<schema::ColumnIdentifier> group_by_fields;

   AggregateNode(QueryNodePtr child, std::vector<schema::ColumnIdentifier> group_by_fields);

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;

   [[nodiscard]] std::string_view getType() const override;
};

}  // namespace silo::query_engine::operators
