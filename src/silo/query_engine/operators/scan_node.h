#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <arrow/result.h>

#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// Leaf node referencing a table by name. Outputs all columns.
/// Must be eliminated during pushdown before query plan generation.
class ScanNode final : public QueryNode {
  public:
   schema::TableName table_name;
   std::vector<schema::ColumnIdentifier> output_schema;

   ScanNode(schema::TableName table_name, std::vector<schema::ColumnIdentifier> output_schema);

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;
};

}  // namespace silo::query_engine::operators
