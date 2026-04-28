#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <arrow/result.h>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// Computes phylogenetic subtree for matching rows.
class PhyloSubtreeNode final : public QueryNode {
  public:
   std::shared_ptr<storage::Table> table;
   std::unique_ptr<filter::expressions::Expression> filter;
   std::string column_name;
   bool print_nodes_not_in_tree;
   bool contract_unary_nodes;
   std::optional<schema::TableName> table_name;

   PhyloSubtreeNode(
      std::shared_ptr<storage::Table> table,
      std::unique_ptr<filter::expressions::Expression> filter,
      std::string column_name,
      bool print_nodes_not_in_tree,
      bool contract_unary_nodes
   );

   PhyloSubtreeNode(
      schema::TableName table_name,
      std::unique_ptr<filter::expressions::Expression> filter,
      std::string column_name,
      bool print_nodes_not_in_tree,
      bool contract_unary_nodes
   );

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;
};

}  // namespace silo::query_engine::operators
