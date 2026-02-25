#pragma once

#include <string>

#include "silo/query_engine/operators/query_node.h"

namespace silo::query_engine::operators {

/// Placeholder for mostRecentCommonAncestor action, resolved during pushdown.
class UnresolvedMostRecentCommonAncestorNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::string column_name;
   bool print_nodes_not_in_tree;

   UnresolvedMostRecentCommonAncestorNode(
      QueryNodePtr child,
      std::string column_name,
      bool print_nodes_not_in_tree
   )
       : child(std::move(child)),
         column_name(std::move(column_name)),
         print_nodes_not_in_tree(print_nodes_not_in_tree) {}

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override {
      return {};
   }

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
      const config::QueryOptions& /*query_options*/
   ) const override {
      throw std::runtime_error(
         "UnresolvedMostRecentCommonAncestorNode must be eliminated during pushdown"
      );
   }
};

}  // namespace silo::query_engine::operators
