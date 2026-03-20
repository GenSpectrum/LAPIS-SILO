#include "silo/query_engine/operators/count_filter_node.h"

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/builder.h>

#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/operators/compute_partition_filters.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

CountFilterNode::CountFilterNode(
   std::shared_ptr<storage::Table> table,
   std::unique_ptr<filter::expressions::Expression> filter
)
    : table(std::move(table)),
      filter(std::move(filter)) {}

std::vector<schema::ColumnIdentifier> CountFilterNode::getOutputSchema() const {
   std::vector<schema::ColumnIdentifier> output_fields;
   output_fields.emplace_back("count", schema::ColumnType::INT64);
   return output_fields;
}

std::string_view CountFilterNode::getType() const {
   return "Aggregated";
}

arrow::Result<PartialArrowPlan> CountFilterNode::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
   const config::QueryOptions& /*query_options*/
) const {
   auto partition_filters = computePartitionFilters(filter, *table);

   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      [partition_filters = std::move(partition_filters),
       already_produced = false]() mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (already_produced) {
         const std::optional<arrow::ExecBatch> result = std::nullopt;
         return arrow::Future{result};
      }
      already_produced = true;

      int64_t result_count = 0;
      for (const auto& partition_filter : partition_filters) {
         result_count += static_cast<int64_t>(partition_filter.getConstReference().cardinality());
      }

      arrow::Int64Builder result_builder{};
      ARROW_RETURN_NOT_OK(result_builder.Append(result_count));
      arrow::Datum datum;
      ARROW_ASSIGN_OR_RAISE(datum, result_builder.Finish());
      ARROW_ASSIGN_OR_RAISE(
         const std::optional<arrow::ExecBatch> result, arrow::ExecBatch::Make({datum})
      );
      return arrow::Future{result};
   };

   ARROW_ASSIGN_OR_RAISE(auto arrow_plan, arrow::acero::ExecPlan::Make());

   std::vector<schema::ColumnIdentifier> output_schema = getOutputSchema();

   const arrow::acero::SourceNodeOptions options{
      exec_node::columnsToArrowSchema(output_schema),
      std::move(producer),
      arrow::Ordering::Implicit()
   };
   ARROW_ASSIGN_OR_RAISE(
      auto node, arrow::acero::MakeExecNode("source", arrow_plan.get(), {}, options)
   );

   return PartialArrowPlan{node, arrow_plan};
}

}  // namespace silo::query_engine::operators
