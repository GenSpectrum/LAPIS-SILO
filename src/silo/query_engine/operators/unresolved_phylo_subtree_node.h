#pragma once

#include <string>

#include "silo/query_engine/operators/query_node.h"

namespace silo::query_engine::operators {

/// Placeholder for phyloSubtree action, resolved during pushdown.
class UnresolvedPhyloSubtreeNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::string column_name;
   bool print_nodes_not_in_tree;
   bool contract_unary_nodes;

   UnresolvedPhyloSubtreeNode(
      QueryNodePtr child,
      std::string column_name,
      bool print_nodes_not_in_tree,
      bool contract_unary_nodes
   )
       : child(std::move(child)),
         column_name(std::move(column_name)),
         print_nodes_not_in_tree(print_nodes_not_in_tree),
         contract_unary_nodes(contract_unary_nodes) {}

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override {
      return {};
   }

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
      const config::QueryOptions& /*query_options*/
   ) const override {
      throw std::runtime_error("UnresolvedPhyloSubtreeNode must be eliminated during pushdown");
   }
};

}  // namespace silo::query_engine::operators
