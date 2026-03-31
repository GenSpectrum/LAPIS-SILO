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
#include "silo/query_engine/operators/compute_filter.h"
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

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Result<PartialArrowPlan> CountFilterNode::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
   const config::QueryOptions& /*query_options*/
) const {
   auto filter_bitmap = computeFilter(filter, *table);

   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      [filter_bitmap = std::move(filter_bitmap),
       already_produced = false]() mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (already_produced) {
         const std::optional<arrow::ExecBatch> result = std::nullopt;
         return arrow::Future{result};
      }
      already_produced = true;

      auto result_count = static_cast<int64_t>(filter_bitmap.getConstReference().cardinality());

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

   const std::vector<schema::ColumnIdentifier> output_schema = getOutputSchema();

   const arrow::acero::SourceNodeOptions options{
      exec_node::columnsToArrowSchema(output_schema),
      std::move(producer),
      arrow::Ordering::Implicit()
   };
   ARROW_ASSIGN_OR_RAISE(
      auto node, arrow::acero::MakeExecNode("source", arrow_plan.get(), {}, options)
   );

   return PartialArrowPlan{.top_node = node, .plan = arrow_plan};
}

}  // namespace silo::query_engine::operators
