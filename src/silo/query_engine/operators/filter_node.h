#pragma once

#include <map>
#include <memory>
#include <vector>

#include <arrow/result.h>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// Applies a filter expression to its child's output.
/// Must be eliminated during pushdown before query plan generation.
class FilterNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::unique_ptr<filter::expressions::Expression> filter;

   FilterNode(QueryNodePtr child, std::unique_ptr<filter::expressions::Expression> filter);

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;
};

}  // namespace silo::query_engine::operators
