#pragma once

#include <string>
#include <vector>

#include "silo/query_engine/operators/query_node.h"

namespace silo::query_engine::operators {

/// Placeholder for insertions action, resolved during pushdown.
template <typename SymbolType>
class UnresolvedInsertionsNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::vector<std::string> sequence_names;

   UnresolvedInsertionsNode(QueryNodePtr child, std::vector<std::string> sequence_names)
       : child(std::move(child)),
         sequence_names(std::move(sequence_names)) {}

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override {
      return {};
   }

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
      const config::QueryOptions& /*query_options*/
   ) const override {
      throw std::runtime_error("UnresolvedInsertionsNode must be eliminated during pushdown");
   }
};

}  // namespace silo::query_engine::operators
