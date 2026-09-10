#pragma once

#include <map>
#include <memory>
#include <string_view>
#include <vector>

#include <arrow/result.h>

#include "rhydb/query_engine/operators/query_node.h"
#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/table.h"

namespace rhydb::query_engine::operators {

/// Source-type node that lists all tables in the database as data rows, one row
/// per table. It takes no input; the table names are read from the database's
/// table map at execution time.
class TablesNode final : public QueryNode {
  public:
   static constexpr std::string_view TABLE_NAME_COLUMN = "tableName";

   TablesNode() = default;

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<arrow::acero::ExecNode*> addToExecPlan(
      arrow::acero::ExecPlan& plan,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;

   [[nodiscard]] NodeKind kind() const override { return NodeKind::TABLES_LIST; }

   [[nodiscard]] nlohmann::json toJson() const override;
};

}  // namespace rhydb::query_engine::operators
