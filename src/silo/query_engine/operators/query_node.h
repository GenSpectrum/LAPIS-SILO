#pragma once

#include <map>
#include <memory>
#include <string_view>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/result.h>

#include "silo/config/runtime_config.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

struct PartialArrowPlan {
   arrow::acero::ExecNode* top_node;
   std::shared_ptr<arrow::acero::ExecPlan> plan;
};

class QueryNode {
  public:
   virtual ~QueryNode() = default;

   [[nodiscard]] virtual arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const = 0;

   [[nodiscard]] virtual std::vector<schema::ColumnIdentifier> getOutputSchema() const = 0;
};

using QueryNodePtr = std::unique_ptr<QueryNode>;

}  // namespace silo::query_engine::operators
