#include "silo/query_engine/operators/filter_node.h"

#include <stdexcept>

namespace silo::query_engine::operators {

FilterNode::FilterNode(QueryNodePtr child, std::unique_ptr<filter::expressions::Expression> filter)
    : child(std::move(child)),
      filter(std::move(filter)) {}

std::vector<schema::ColumnIdentifier> FilterNode::getOutputSchema() const {
   return child->getOutputSchema();
}

arrow::Result<PartialArrowPlan> FilterNode::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
   const config::QueryOptions& /*query_options*/
) const {
   throw std::runtime_error(
      "FilterNode must be eliminated during pushdown before query plan generation"
   );
}

}  // namespace silo::query_engine::operators
