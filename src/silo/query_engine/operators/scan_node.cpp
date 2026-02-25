#include "silo/query_engine/operators/scan_node.h"

#include <stdexcept>

#include <fmt/format.h>

namespace silo::query_engine::operators {

ScanNode::ScanNode(
   schema::TableName table_name,
   std::vector<schema::ColumnIdentifier> output_schema
)
    : table_name(std::move(table_name)),
      output_schema(std::move(output_schema)) {}

std::vector<schema::ColumnIdentifier> ScanNode::getOutputSchema() const {
   return output_schema;
}

arrow::Result<PartialArrowPlan> ScanNode::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
   const config::QueryOptions& /*query_options*/
) const {
   throw std::runtime_error(fmt::format(
      "ScanNode('{}') must be eliminated during pushdown before query plan generation",
      table_name.getName()
   ));
}

}  // namespace silo::query_engine::operators
