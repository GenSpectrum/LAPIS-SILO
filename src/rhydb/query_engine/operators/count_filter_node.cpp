#include "rhydb/query_engine/operators/count_filter_node.h"

#include <map>
#include <memory>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <nlohmann/json.hpp>

#include "rhydb/query_engine/exec_node/arrow_util.h"
#include "rhydb/query_engine/exec_node/count_filter.h"
#include "rhydb/query_engine/operators/compute_filter.h"
#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/table.h"

namespace rhydb::query_engine::operators {

CountFilterNode::CountFilterNode(
   std::shared_ptr<storage::Table> table,
   std::unique_ptr<scalar_expressions::ScalarExpression> filter,
   std::string output_name
)
    : table(std::move(table)),
      filter(std::move(filter)),
      output_name(std::move(output_name)) {}

std::vector<schema::ColumnIdentifier> CountFilterNode::getOutputSchema() const {
   std::vector<schema::ColumnIdentifier> output_fields;
   output_fields.emplace_back(output_name, schema::ColumnType::INT64);
   return output_fields;
}

arrow::Result<arrow::acero::ExecNode*> CountFilterNode::addToExecPlan(
   arrow::acero::ExecPlan& plan,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
   const config::QueryOptions& /*query_options*/
) const {
   return exec_node::addCountFilterNode(
      plan, CompiledFilter{table, *filter}, exec_node::columnsToArrowSchema(getOutputSchema())
   );
}

nlohmann::json CountFilterNode::toJson() const {
   return {
      {"type", nodeKindToString(kind())},
      {"table", table->logTable()},
      {"filter", filter->toString()},
      {"output_name", output_name},
   };
}

}  // namespace rhydb::query_engine::operators
