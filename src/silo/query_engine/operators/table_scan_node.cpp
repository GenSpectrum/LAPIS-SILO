#include "silo/query_engine/operators/table_scan_node.h"

#include "silo/query_engine/exec_node/table_scan.h"
#include "silo/query_engine/operators/compute_partition_filters.h"

namespace silo::query_engine::operators {

TableScanNode::TableScanNode(
   std::shared_ptr<storage::Table> table,
   std::unique_ptr<filter::expressions::Expression> filter,
   std::vector<schema::ColumnIdentifier> fields
)
    : table(std::move(table)),
      filter(std::move(filter)),
      fields(std::move(fields)) {}

std::vector<schema::ColumnIdentifier> TableScanNode::getOutputSchema() const {
   return fields;
}

arrow::Result<PartialArrowPlan> TableScanNode::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
   const config::QueryOptions& query_options
) const {
   auto partition_filters = computePartitionFilters(filter, *table);

   ARROW_ASSIGN_OR_RAISE(auto arrow_plan, arrow::acero::ExecPlan::Make());

   ARROW_ASSIGN_OR_RAISE(
      arrow::acero::ExecNode * node,
      exec_node::makeTableScan(
         arrow_plan.get(),
         fields,
         std::move(partition_filters),
         table,
         query_options.materialization_cutoff
      )
   );

   return PartialArrowPlan{.top_node = node, .plan = arrow_plan};
}

}  // namespace silo::query_engine::operators
