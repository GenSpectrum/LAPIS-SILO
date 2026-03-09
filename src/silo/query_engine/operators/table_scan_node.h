#pragma once

#include <map>
#include <memory>
#include <string_view>
#include <vector>

#include <arrow/result.h>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// Leaf operator: scans a table applying a filter expression to produce matching rows.
class TableScanNode final : public QueryNode {
  public:
   std::shared_ptr<storage::Table> table;
   std::unique_ptr<filter::expressions::Expression> filter;
   std::vector<schema::ColumnIdentifier> fields;

   TableScanNode(
      std::shared_ptr<storage::Table> table,
      std::unique_ptr<filter::expressions::Expression> filter,
      std::vector<schema::ColumnIdentifier> fields
   );

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;
};

}  // namespace silo::query_engine::operators
