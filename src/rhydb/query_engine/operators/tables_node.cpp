#include "rhydb/query_engine/operators/tables_node.h"

#include <map>
#include <memory>
#include <string>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/builder.h>
#include <arrow/table.h>
#include <nlohmann/json.hpp>

#include "rhydb/query_engine/exec_node/arrow_util.h"
#include "rhydb/schema/database_schema.h"

namespace rhydb::query_engine::operators {

using config::QueryOptions;
using schema::ColumnIdentifier;
using schema::ColumnType;
using schema::TableName;
using storage::Table;

std::vector<ColumnIdentifier> TablesNode::getOutputSchema() const {
   return {
      {.name = std::string{TABLE_NAME_COLUMN}, .type = ColumnType::STRING},
   };
}

arrow::Result<arrow::acero::ExecNode*> TablesNode::addToExecPlan(
   arrow::acero::ExecPlan& plan,
   const std::map<TableName, std::shared_ptr<Table>>& tables,
   const QueryOptions& /*query_options*/
) const {
   arrow::StringBuilder table_name_builder{};
   for (const auto& [table_name, _] : tables) {
      ARROW_RETURN_NOT_OK(table_name_builder.Append(table_name.getName()));
   }
   std::shared_ptr<arrow::Array> table_name_array;
   ARROW_ASSIGN_OR_RAISE(table_name_array, table_name_builder.Finish());

   const auto arrow_schema = exec_node::columnsToArrowSchema(getOutputSchema());
   const auto table = arrow::Table::Make(arrow_schema, {table_name_array});

   const arrow::acero::TableSourceNodeOptions options{table};
   return arrow::acero::MakeExecNode("table_source", &plan, {}, options);
}

nlohmann::json TablesNode::toJson() const {
   return {
      {"type", nodeKindToString(kind())},
   };
}

}  // namespace rhydb::query_engine::operators
