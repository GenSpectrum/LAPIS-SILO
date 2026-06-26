#include "silo/query_engine/operators/schema_node.h"

#include <memory>
#include <string>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/builder.h>
#include <arrow/compute/api.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::operators {

SchemaNode::SchemaNode(QueryNodePtr child)
    : child(std::move(child)) {}

std::vector<schema::ColumnIdentifier> SchemaNode::getOutputSchema() const {
   return {
      {std::string(FIELD_NAME_COLUMN), schema::ColumnType::STRING},
      {std::string(TYPE_COLUMN), schema::ColumnType::STRING},
   };
}

arrow::Result<arrow::acero::ExecNode*> SchemaNode::addToExecPlan(
   arrow::acero::ExecPlan& plan,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
   const config::QueryOptions& /*query_options*/
) const {
   const auto child_schema = child->getOutputSchema();

   arrow::StringBuilder field_name_builder;
   arrow::StringBuilder type_builder;

   for (const auto& col : child_schema) {
      ARROW_RETURN_NOT_OK(field_name_builder.Append(col.name));
      ARROW_RETURN_NOT_OK(
         type_builder.Append(std::string(schema::columnTypeToString(col.type)))
      );
   }

   ARROW_ASSIGN_OR_RAISE(auto field_name_array, field_name_builder.Finish());
   ARROW_ASSIGN_OR_RAISE(auto type_array, type_builder.Finish());

   ARROW_ASSIGN_OR_RAISE(
      auto exec_batch,
      arrow::compute::ExecBatch::Make({arrow::Datum(field_name_array), arrow::Datum(type_array)})
   );

   auto batch_holder =
      std::make_shared<std::pair<arrow::ExecBatch, bool>>(std::move(exec_batch), false);
   auto generator = [batch_holder]() mutable
      -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (!batch_holder->second) {
         batch_holder->second = true;
         return arrow::Future<std::optional<arrow::ExecBatch>>::MakeFinished(batch_holder->first);
      }
      return arrow::Future<std::optional<arrow::ExecBatch>>::MakeFinished(std::nullopt);
   };

   const auto arrow_schema = exec_node::columnsToArrowSchema(getOutputSchema());
   const arrow::acero::SourceNodeOptions source_options(
      arrow_schema, std::move(generator), arrow::Ordering::Implicit()
   );
   return arrow::acero::MakeExecNode("source", &plan, {}, source_options);
}

nlohmann::json SchemaNode::toJson() const {
   return {
      {"type", nodeKindToString(kind())},
      {"child", child->toJson()},
   };
}

}  // namespace silo::query_engine::operators
