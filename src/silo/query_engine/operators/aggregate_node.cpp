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

using silo::query_engine::operators::AggregateDefinition;
using silo::query_engine::operators::AggregateFunction;

std::string arrowFunctionName(AggregateFunction func, bool has_groups) {
   switch (func) {
      case AggregateFunction::COUNT:
         return has_groups ? "hash_count_all" : "count_all";
   }
   SILO_UNREACHABLE();
}

arrow::acero::AggregateNodeOptions buildAggregateOptions(
   const std::vector<silo::schema::ColumnIdentifier>& group_by_fields,
   const std::vector<AggregateDefinition>& aggregates,
   const arrow::Schema& input_schema
) {
   const bool has_groups = !group_by_fields.empty();

   std::vector<arrow::compute::Aggregate> arrow_aggregates;
   arrow_aggregates.reserve(aggregates.size());

   for (const auto& agg : aggregates) {
      std::vector<arrow::FieldRef> source_refs;
      std::shared_ptr<arrow::compute::FunctionOptions> options;

      switch (agg.function) {
         case AggregateFunction::COUNT: {
            options = std::make_shared<arrow::compute::CountOptions>(
               arrow::compute::CountOptions::CountMode::ALL
            );
            break;
         }
      }

      arrow_aggregates.emplace_back(
         arrowFunctionName(agg.function, has_groups),
         options,
         std::move(source_refs),
         agg.output_name
      );
   }

   if (!has_groups) {
      return arrow::acero::AggregateNodeOptions(std::move(arrow_aggregates));
   }

   std::vector<arrow::FieldRef> field_refs;
   field_refs.reserve(group_by_fields.size());
   for (const auto& field : group_by_fields) {
      SILO_ASSERT(input_schema.CanReferenceFieldByName(field.name).ok());
      field_refs.emplace_back(field.name);
   }

   return arrow::acero::AggregateNodeOptions(std::move(arrow_aggregates), std::move(field_refs));
}

}  // namespace

namespace silo::query_engine::operators {

AggregateNode::AggregateNode(
   QueryNodePtr child,
   std::vector<schema::ColumnIdentifier> group_by_fields,
   std::vector<AggregateDefinition> aggregates
)
    : child(std::move(child)),
      group_by_fields(std::move(group_by_fields)),
      aggregates(std::move(aggregates)) {}

std::vector<schema::ColumnIdentifier> AggregateNode::getOutputSchema() const {
   auto output_fields = group_by_fields;
   for (const auto& agg : aggregates) {
      schema::ColumnType type;
      switch (agg.function) {
         case AggregateFunction::COUNT:
            type = schema::ColumnType::INT64;
            break;
      }
      output_fields.emplace_back(agg.output_name, type);
   }
   return output_fields;
}

arrow::Result<PartialArrowPlan> AggregateNode::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options
) const {
   ARROW_ASSIGN_OR_RAISE(auto plan, child->toQueryPlan(tables, query_options));

   auto input_schema = plan.top_node->output_schema();

   const arrow::acero::AggregateNodeOptions aggregate_node_options =
      buildAggregateOptions(group_by_fields, aggregates, *input_schema);

   ARROW_ASSIGN_OR_RAISE(
      plan.top_node,
      arrow::acero::MakeExecNode(
         "aggregate", plan.plan.get(), {plan.top_node}, aggregate_node_options
      )
   );

   return plan;
}

}  // namespace silo::query_engine::operators
