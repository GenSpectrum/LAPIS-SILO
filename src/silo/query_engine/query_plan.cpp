#include "silo/query_engine/query_plan.h"

#include <arrow/acero/query_context.h>
#include <arrow/array.h>
#include <arrow/record_batch.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "evobench/evobench.hpp"
#include "silo/query_engine/exec_node/ndjson_sink.h"

namespace silo::query_engine {

arrow::Result<QueryPlan> QueryPlan::makeQueryPlan(
   std::shared_ptr<arrow::acero::ExecPlan> arrow_plan,
   arrow::acero::ExecNode* root,
   std::string_view request_id
) {
   QueryPlan query_plan{arrow_plan, request_id};
   ARROW_ASSIGN_OR_RAISE(
      query_plan.backpressure_monitor,
      exec_node::createGenerator(arrow_plan.get(), root, &query_plan.results_generator)
   );
   query_plan.results_schema = root->output_schema();
   ARROW_RETURN_NOT_OK(query_plan.arrow_plan->Validate());
   return query_plan;
}

arrow::Status QueryPlan::executeAndWriteImpl(
   std::ostream* output_stream,
   uint64_t timeout_in_seconds
) {
   EVOBENCH_SCOPE("QueryPlan", "execute");
   SPDLOG_TRACE("{}", arrow_plan->ToString());
   SPDLOG_DEBUG("Request Id [{}] - QueryPlan - Starting the plan.", request_id);
   arrow_plan->StartProducing();
   SPDLOG_DEBUG(
      "Request Id [{}] - QueryPlan - Plan started producing, will now read the resulting batches.",
      request_id
   );

   // Ensure plan is stopped on any exit path (timeout/error/exception).
   struct PlanStopGuard {
      std::string_view request_id;
      std::shared_ptr<arrow::acero::ExecPlan> plan;

      ~PlanStopGuard() {
         constexpr double GRACE_SHUTDOWN_SECONDS = 5.0;  // avoid hanging on teardown
         try {
            if (plan) {
               SPDLOG_DEBUG(
                  "Request Id [{}] - QueryPlan - Stopping arrow execution plan", request_id
               );
               plan->StopProducing();
               auto finished_future = plan->finished();
               const bool drained = finished_future.Wait(GRACE_SHUTDOWN_SECONDS);
               if (!drained) {
                  SPDLOG_WARN(
                     "Request Id [{}] - QueryPlan - ExecPlan cleanup exceeded {} s grace; "
                     "continuing.",
                     request_id,
                     GRACE_SHUTDOWN_SECONDS
                  );
               }
            }
         } catch (const std::exception& e) {
            SPDLOG_ERROR(
               "Request Id [{}] - QueryPlan - Error while tearing down Arrow::acero::ExecPlan: {}",
               request_id,
               e.what()
            );
         } catch (...) {
            SPDLOG_ERROR(
               "Request Id [{}] - QueryPlan - Unknown non-std::exception error while tearing down "
               "Arrow::acero::ExecPlan.",
               request_id
            );
         }
      }
   } guard{request_id, arrow_plan};

   while (true) {
      arrow::Future<std::optional<arrow::ExecBatch>> future_batch = results_generator();
      SPDLOG_DEBUG("Request Id [{}] - QueryPlan - await the next batch", request_id);
      bool finished_batch_in_time = future_batch.Wait(static_cast<double>(timeout_in_seconds));
      if (!finished_batch_in_time) {
         SPDLOG_WARN(
            "Request Id [{}] - QueryPlan - Batch wait timed out after {} s — stopping plan.",
            request_id,
            timeout_in_seconds
         );
         return arrow::Status::ExecutionError(
            fmt::format("Request timed out, no batch within {} seconds.", timeout_in_seconds)
         );
      }
      ARROW_ASSIGN_OR_RAISE(std::optional<arrow::ExecBatch> optional_batch, future_batch.result());
      SPDLOG_DEBUG("Request Id [{}] - QueryPlan - Batch received", request_id);
      SPDLOG_DEBUG(
         "Request Id [{}] - QueryPlan - Current backpressure size: {} bytes, operation is {}",
         request_id,
         backpressure_monitor->bytes_in_use(),
         backpressure_monitor->is_paused() ? "paused" : "running"
      );

      if (!optional_batch.has_value()) {
         break;  // end of input
      }
      SPDLOG_DEBUG(
         "Request Id [{}] - QueryPlan - Batch contains data with {} values.",
         request_id,
         optional_batch.value().length
      );

      // TODO(#764) make output format configurable
      ARROW_RETURN_NOT_OK(
         exec_node::writeBatchAsNdjson(optional_batch.value(), results_schema, output_stream)
      );
   };
   SPDLOG_DEBUG("Request Id [{}] - QueryPlan - Finished reading all batches.", request_id);
   return arrow::Status::OK();
}

void QueryPlan::executeAndWrite(std::ostream* output_stream, uint64_t timeout_in_seconds) {
   auto status = executeAndWriteImpl(output_stream, timeout_in_seconds);
   if (!status.ok()) {
      if (status.IsIOError()) {
         SPDLOG_WARN(
            "The request {} encountered an IO Error when sending the response. We expect that the "
            "user cancelled the request while the response was send and ignore the error",
            request_id
         );
      } else {
         throw std::runtime_error(fmt::format(
            "Request Id [{}] - Internal server error. Please notify developers. SILO likely "
            "constructed an invalid arrow plan and more user-input validation needs to be added: "
            "{}",
            request_id,
            status.message()
         ));
      }
   }
}

}  // namespace silo::query_engine
