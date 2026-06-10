#include "silo/query_engine/operators/union_all_node.h"

#include <functional>
#include <optional>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/util/async_generator.h>
#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

#include "silo/common/panic.h"
#include "silo/query_engine/exec_node/arrow_util.h"

namespace silo::query_engine::operators {

namespace {

/// Execute a child plan to completion and collect all output batches.
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Result<std::vector<arrow::ExecBatch>> drainChildPlan(PartialArrowPlan& child_plan) {
   arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> generator;
   arrow::acero::BackpressureMonitor* backpressure_monitor = nullptr;
   const arrow::acero::SinkNodeOptions sink_options{
      &generator, arrow::acero::BackpressureOptions{}, &backpressure_monitor, true
   };
   ARROW_ASSIGN_OR_RAISE(
      auto sink_node,
      arrow::acero::MakeExecNode("sink", child_plan.plan.get(), {child_plan.top_node}, sink_options)
   );
   (void)sink_node;
   (void)backpressure_monitor;

   ARROW_RETURN_NOT_OK(child_plan.plan->Validate());
   child_plan.plan->StartProducing();

   std::vector<arrow::ExecBatch> batches;
   while (true) {
      auto future = generator();
      auto result = future.result();
      if (!result.ok()) {
         // The plan may report Cancelled when torn down after all data was produced
         // due to Arrow Acero's internal threading. This is expected and not an error.
         if (result.status().IsCancelled()) {
            SPDLOG_WARN(
               "UnionAll child plan generator returned Cancelled after {} batches",
               batches.size()
            );
            break;
         }
         return result.status();
      }
      if (!result->has_value()) {
         break;
      }
      batches.push_back(std::move(result->value()));
   }

   // Ensure the plan is fully wound down before returning.
   auto finished_future = child_plan.plan->finished();
   if (!finished_future.is_finished()) {
      child_plan.plan->StopProducing();
      finished_future.Wait();
   }

   return batches;
}

arrow::Result<arrow::acero::ExecNode*> makeUnionSource(
   arrow::acero::ExecPlan* plan,
   const std::vector<schema::ColumnIdentifier>& output_schema,
   std::shared_ptr<std::vector<std::vector<arrow::ExecBatch>>> shared_batches
) {
   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      [shared_batches = std::move(shared_batches), child_idx = size_t{0},
       batch_idx = size_t{0}]() mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      while (child_idx < shared_batches->size()) {
         auto& child_batches = (*shared_batches)[child_idx];
         if (batch_idx < child_batches.size()) {
            auto batch = std::move(child_batches[batch_idx]);
            ++batch_idx;
            return arrow::Future<std::optional<arrow::ExecBatch>>{
               std::optional<arrow::ExecBatch>{std::move(batch)}
            };
         }
         ++child_idx;
         batch_idx = 0;
      }
      return arrow::Future<std::optional<arrow::ExecBatch>>{
         std::optional<arrow::ExecBatch>{std::nullopt}
      };
   };

   const arrow::acero::SourceNodeOptions source_options{
      exec_node::columnsToArrowSchema(output_schema),
      std::move(producer),
      arrow::Ordering::Implicit()
   };
   return arrow::acero::MakeExecNode("source", plan, {}, source_options);
}

}  // namespace

UnionAllNode::UnionAllNode(std::vector<QueryNodePtr> children)
    : children(std::move(children)) {
   SILO_ASSERT(this->children.size() == 2);
}

std::vector<schema::ColumnIdentifier> UnionAllNode::getOutputSchema() const {
   return children.front()->getOutputSchema();
}

arrow::Result<PartialArrowPlan> UnionAllNode::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options
) const {
   // NOTE: This eagerly materializes ALL child batches into memory before producing output.
   // For large datasets both child results are buffered simultaneously. A future optimization
   // could stream batches lazily, draining the second child only after the first is consumed.
   auto all_batches = std::make_shared<std::vector<std::vector<arrow::ExecBatch>>>();
   all_batches->reserve(children.size());

   for (const auto& child : children) {
      ARROW_ASSIGN_OR_RAISE(auto child_plan, child->toQueryPlan(tables, query_options));
      ARROW_ASSIGN_OR_RAISE(auto batches, drainChildPlan(child_plan));
      all_batches->push_back(std::move(batches));
   }

   ARROW_ASSIGN_OR_RAISE(auto arrow_plan, arrow::acero::ExecPlan::Make());

   ARROW_ASSIGN_OR_RAISE(
      auto source_node,
      makeUnionSource(arrow_plan.get(), getOutputSchema(), std::move(all_batches))
   );

   return PartialArrowPlan{.top_node = source_node, .plan = arrow_plan};
}

nlohmann::json UnionAllNode::toJson() const {
   nlohmann::json children_json = nlohmann::json::array();
   for (const auto& child : children) {
      children_json.push_back(child->toJson());
   }
   return {
      {"type", nodeKindToString(kind())},
      {"children", std::move(children_json)},
   };
}

}  // namespace silo::query_engine::operators
