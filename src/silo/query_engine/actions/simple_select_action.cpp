#include "silo/query_engine/actions/simple_select_action.h"

#include <ranges>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/compute/expression.h>
#include <fmt/ranges.h>

#include "evobench/evobench.hpp"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/exec_node/table_scan.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::actions {

void SimpleSelectAction::validateOrderByFields(const schema::TableSchema& schema) const {
   auto output_schema = getOutputSchema(schema);
   auto output_schema_fields =
      output_schema |
      std::views::transform([](const auto& identifier) { return identifier.name; }) |
      std::views::common;
   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         std::ranges::find(output_schema_fields, field.name) != std::end(output_schema_fields),
         "OrderByField {} is not contained in the result of this operation. "
         "The only fields returned by this action are {}",
         field.name,
         fmt::join(output_schema_fields, ", ")
      )
   }
}

arrow::Result<QueryPlan> SimpleSelectAction::toQueryPlanImpl(
   std::shared_ptr<const storage::Table> table,
   std::vector<CopyOnWriteBitmap> partition_filters,
   const config::QueryOptions& query_options,
   std::string_view request_id
) const {
   EVOBENCH_SCOPE("Select", "toQueryPlanImpl");
   ARROW_ASSIGN_OR_RAISE(auto arrow_plan, arrow::acero::ExecPlan::Make());

   arrow::acero::ExecNode* node;
   ARROW_ASSIGN_OR_RAISE(
      node,
      exec_node::makeTableScan(
         arrow_plan.get(),
         getOutputSchema(table->schema),
         std::move(partition_filters),
         table,
         query_options.materialization_cutoff
      )
   );

   ARROW_ASSIGN_OR_RAISE(node, addOrderingNodes(arrow_plan.get(), node, table->schema));

   ARROW_ASSIGN_OR_RAISE(node, addLimitAndOffsetNode(arrow_plan.get(), node));

   ARROW_ASSIGN_OR_RAISE(node, addZstdDecompressNode(arrow_plan.get(), node, table->schema));

   return QueryPlan::makeQueryPlan(arrow_plan, node, request_id);
}

}  // namespace silo::query_engine::actions