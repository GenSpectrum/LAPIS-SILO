#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <arrow/result.h>

#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

enum class AggregateFunction : uint8_t { COUNT };

struct AggregateDefinition {
   std::string output_name;
   AggregateFunction function;
   std::optional<std::string> source_column;
};

class AggregateNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::vector<schema::ColumnIdentifier> group_by_fields;
   std::vector<AggregateDefinition> aggregates;

   AggregateNode(
      QueryNodePtr child,
      std::vector<schema::ColumnIdentifier> group_by_fields,
      std::vector<AggregateDefinition> aggregates
   );

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;
};

}  // namespace silo::query_engine::operators
