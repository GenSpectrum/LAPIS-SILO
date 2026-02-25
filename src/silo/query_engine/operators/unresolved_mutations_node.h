#pragma once

#include <string>
#include <vector>

#include "silo/query_engine/operators/query_node.h"

namespace silo::query_engine::operators {

/// Placeholder for mutations action, resolved during pushdown.
template <typename SymbolType>
class UnresolvedMutationsNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::vector<std::string> sequence_names;
   double min_proportion;
   std::vector<std::string> fields;

   UnresolvedMutationsNode(
      QueryNodePtr child,
      std::vector<std::string> sequence_names,
      double min_proportion,
      std::vector<std::string> fields
   )
       : child(std::move(child)),
         sequence_names(std::move(sequence_names)),
         min_proportion(min_proportion),
         fields(std::move(fields)) {}

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override {
      return {};
   }

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
      const config::QueryOptions& /*query_options*/
   ) const override {
      throw std::runtime_error("UnresolvedMutationsNode must be eliminated during pushdown");
   }
};

}  // namespace silo::query_engine::operators
