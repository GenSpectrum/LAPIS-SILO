#include "silo/query_engine/operators/union_all_node.h"

#include <functional>
#include <optional>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/util/async_generator.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/common/panic.h"
#include "silo/common/size_constants.h"
#include "silo/query_engine/exec_node/arrow_util.h"

namespace silo::query_engine::operators {

namespace {

/// Mutable state for the streaming union producer.
/// Owns the pre-built child plans and drains them lazily one at a time.
struct StreamState {
   std::vector<PartialArrowPlan> child_plans;

   size_t child_idx = 0;
   std::shared_ptr<arrow::acero::ExecPlan> current_plan;
   arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> current_generator;
   bool generator_active = false;

   ~StreamState() { stopCurrentChild(); }

   /// Attach a sink to the current child plan and start producing.
   arrow::Status startCurrentChild() {
      SILO_ASSERT(child_idx < child_plans.size());
      SILO_ASSERT(!generator_active);

      auto& partial = child_plans[child_idx];
      current_plan = partial.plan;

      arrow::acero::BackpressureMonitor* backpressure_monitor = nullptr;
      const arrow::acero::SinkNodeOptions sink_options{
         &current_generator,
         arrow::acero::BackpressureOptions{
            /*.resume_if_below =*/silo::common::S_16_KB,
            /*.pause_if_above =*/silo::common::S_64_MB
         },
         &backpressure_monitor,
         true
      };
      ARROW_ASSIGN_OR_RAISE(
         auto sink_node,
         arrow::acero::MakeExecNode("sink", current_plan.get(), {partial.top_node}, sink_options)
      );
      (void)sink_node;
      (void)backpressure_monitor;

      ARROW_RETURN_NOT_OK(current_plan->Validate());
      current_plan->StartProducing();
      generator_active = true;
      return arrow::Status::OK();
   }

   /// Tear down the current child plan.
   void stopCurrentChild() {
      constexpr double GRACE_SHUTDOWN_SECONDS = 5.0;
      if (!current_plan) {
         return;
      }
      auto finished_future = current_plan->finished();
      if (!finished_future.is_finished()) {
         current_plan->StopProducing();
         const bool drained = finished_future.Wait(GRACE_SHUTDOWN_SECONDS);
         if (!drained) {
            SPDLOG_WARN(
               "UnionAll: child plan cleanup exceeded {} s grace; continuing.",
               GRACE_SHUTDOWN_SECONDS
            );
         }
      }
      current_plan.reset();
      generator_active = false;
   }
};

/// Pull the next batch from the streaming union.
/// Starts child plans on demand, drains them one at a time, then advances.
/// Note: blocks synchronously on the child generator. This is intentional — the child
/// plan runs on the same shared Arrow thread pool, so async continuations would deadlock.
/// This matches the pattern used by QueryPlan::executeAndWriteImpl.
arrow::Future<std::optional<arrow::ExecBatch>> pullNext(const std::shared_ptr<StreamState>& state) {
   while (true) {
      if (state->child_idx >= state->child_plans.size()) {
         return arrow::Future{std::optional<arrow::ExecBatch>{std::nullopt}};
      }

      if (!state->generator_active) {
         auto status = state->startCurrentChild();
         if (!status.ok()) {
            return arrow::Future{arrow::Result<std::optional<arrow::ExecBatch>>{status}};
         }
      }

      auto future = state->current_generator();
      auto result = future.result();

      if (!result.ok()) {
         // Propagate all errors (including Cancelled) to the caller.
         state->stopCurrentChild();
         return arrow::Future{arrow::Result<std::optional<arrow::ExecBatch>>{result.status()}};
      }

      if (!result->has_value()) {
         state->stopCurrentChild();
         ++state->child_idx;
         continue;
      }

      return arrow::Future{std::move(result.ValueUnsafe())};
   }
}

}  // namespace

UnionAllNode::UnionAllNode(QueryNodePtr left, QueryNodePtr right)
    : left(std::move(left)),
      right(std::move(right)) {}

std::vector<schema::ColumnIdentifier> UnionAllNode::getOutputSchema() const {
   return left->getOutputSchema();
}

arrow::Result<PartialArrowPlan> UnionAllNode::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options
) const {
   // Build child plans eagerly (cheap: creates Arrow nodes + compiles bitmap filters).
   // Actual data production is deferred — each child is started and drained lazily
   // by the streaming producer, so only one child's batches are in flight at a time.
   auto state = std::make_shared<StreamState>();
   state->child_plans.reserve(2);
   ARROW_ASSIGN_OR_RAISE(auto left_plan, left->toQueryPlan(tables, query_options));
   state->child_plans.push_back(std::move(left_plan));
   ARROW_ASSIGN_OR_RAISE(auto right_plan, right->toQueryPlan(tables, query_options));
   state->child_plans.push_back(std::move(right_plan));

   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer = [state] {
      return pullNext(state);
   };

   ARROW_ASSIGN_OR_RAISE(auto arrow_plan, arrow::acero::ExecPlan::Make());

   const arrow::acero::SourceNodeOptions source_options{
      exec_node::columnsToArrowSchema(getOutputSchema()),
      std::move(producer),
      arrow::Ordering::Implicit()
   };
   ARROW_ASSIGN_OR_RAISE(
      auto source_node, arrow::acero::MakeExecNode("source", arrow_plan.get(), {}, source_options)
   );

   return PartialArrowPlan{.top_node = source_node, .plan = arrow_plan};
}

nlohmann::json UnionAllNode::toJson() const {
   return {
      {"type", nodeKindToString(kind())},
      {"left", left->toJson()},
      {"right", right->toJson()},
   };
}

}  // namespace silo::query_engine::operators
