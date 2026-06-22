#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <arrow/result.h>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// Finds most recent common ancestor for matching rows.
class MostRecentCommonAncestorNode final : public QueryNode {
  public:
   std::shared_ptr<storage::Table> table;
   std::unique_ptr<expressions::Expression> filter;
   std::string column_name;
   bool print_nodes_not_in_tree;

   MostRecentCommonAncestorNode(
      std::shared_ptr<storage::Table> table,
      std::unique_ptr<expressions::Expression> filter,
      std::string column_name,
      bool print_nodes_not_in_tree
   );

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<arrow::acero::ExecNode*> addToExecPlan(
      arrow::acero::ExecPlan& plan,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;

   [[nodiscard]] NodeKind kind() const override { return NodeKind::MOST_RECENT_COMMON_ANCESTOR; }

   [[nodiscard]] nlohmann::json toJson() const override;
};

}  // namespace silo::query_engine::operators
