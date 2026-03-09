#include "silo/query_engine/operators/aggregate_node.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/compute/api.h>

#include "silo/common/panic.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace {

arrow::acero::AggregateNodeOptions getAggregateOptionsForGroupByFields(
   const std::vector<silo::schema::ColumnIdentifier>& group_by_fields,
   const arrow::Schema& input_schema
) {
   if (group_by_fields.empty()) {
      auto count_options =
         std::make_shared<arrow::compute::CountOptions>(arrow::compute::CountOptions::CountMode::ALL
         );
      const arrow::compute::Aggregate aggregate{
         "count_all", count_options, std::vector<arrow::FieldRef>{}, std::string("count")
      };
      return arrow::acero::AggregateNodeOptions({aggregate});
   }

   std::vector<arrow::FieldRef> field_refs;
   field_refs.reserve(group_by_fields.size());
   for (const auto& field : group_by_fields) {
      SILO_ASSERT(input_schema.CanReferenceFieldByName(field.name).ok());
      field_refs.emplace_back(field.name);
   }

   auto count_options =
      std::make_shared<arrow::compute::CountOptions>(arrow::compute::CountOptions::CountMode::ALL);
   const arrow::compute::Aggregate aggregate{
      "hash_count_all", count_options, std::vector<arrow::FieldRef>{}, std::string("count")
   };
   return arrow::acero::AggregateNodeOptions({aggregate}, field_refs);
}

}  // namespace

namespace silo::query_engine::operators {

AggregateNode::AggregateNode(
   QueryNodePtr child,
   std::vector<schema::ColumnIdentifier> group_by_fields
)
    : child(std::move(child)),
      group_by_fields(std::move(group_by_fields)) {}

std::vector<schema::ColumnIdentifier> AggregateNode::getOutputSchema() const {
   auto output_fields = group_by_fields;
   output_fields.emplace_back("count", schema::ColumnType::INT64);
   return output_fields;
}

arrow::Result<PartialArrowPlan> AggregateNode::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options
) const {
   ARROW_ASSIGN_OR_RAISE(auto plan, child->toQueryPlan(tables, query_options));

   auto input_schema = plan.top_node->output_schema();

   const arrow::acero::AggregateNodeOptions aggregate_node_options =
      getAggregateOptionsForGroupByFields(group_by_fields, *input_schema);

   ARROW_ASSIGN_OR_RAISE(
      plan.top_node,
      arrow::acero::MakeExecNode(
         "aggregate", plan.plan.get(), {plan.top_node}, aggregate_node_options
      )
   );

   return plan;
}

}  // namespace silo::query_engine::operators
