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
   arrow::acero::ExecNode* root
) {
   QueryPlan query_plan{arrow_plan};
   ARROW_RETURN_NOT_OK(
      exec_node::createGenerator(arrow_plan.get(), root, &query_plan.results_generator)
   );
   query_plan.results_schema = root->output_schema();
   ARROW_RETURN_NOT_OK(query_plan.arrow_plan->Validate());
   return query_plan;
}

arrow::Status QueryPlan::executeAndWriteImpl(std::ostream* output_stream) {
   EVOBENCH_SCOPE("QueryPlan", "execute");
   SPDLOG_TRACE("{}", arrow_plan->ToString());
   arrow_plan->StartProducing();
   SPDLOG_TRACE("Plan started producing, will now read the resulting batches.");
   while (true) {
      arrow::Future<std::optional<arrow::ExecBatch>> future_batch = results_generator();
      SPDLOG_TRACE("await the next batch");
      ARROW_ASSIGN_OR_RAISE(std::optional<arrow::ExecBatch> optional_batch, future_batch.result());
      SPDLOG_TRACE("Batch received");

      if (!optional_batch.has_value()) {
         break;  // end of input
      }
      SPDLOG_TRACE("Batch contains data with {} values.", optional_batch.value().length);

      // TODO(#764) make output format configurable
      ARROW_RETURN_NOT_OK(
         exec_node::writeBatchAsNdjson(optional_batch.value(), results_schema, output_stream)
      );
   };
   SPDLOG_TRACE("Finished reading all batches.");
   auto future = arrow_plan->finished();
   if (future.state() == arrow::FutureState::PENDING) {
      SPDLOG_DEBUG(
         "Plan still pending, but no more results in the future? Not a problem in itself as it "
         "could be a race condition so the next await will clear this, but be a hint in case we "
         "detect some weird behavior."
      );
   }
   future.Wait();
   if (future.status().ok()) {
      SPDLOG_DEBUG("All results successfully produced. Clearing ExecPlan.");
   } else {
      return future.status();
   }
   arrow_plan->StopProducing();
   return arrow::Status::OK();
}

void QueryPlan::executeAndWrite(std::ostream* output_stream) {
   auto status = executeAndWriteImpl(output_stream);
   if (!status.ok()) {
      if (status.IsIOError()) {
         SPDLOG_ERROR("The request {} encountered an IO Error when sending the response");
      } else {
         throw std::runtime_error(fmt::format(
            "Internal server error. Please notify developers. SILO likely constructed an invalid "
            "arrow plan and more user-input validation needs to be added: {}",
            status.ToString()
         ));
      }
   }
}

}  // namespace silo::query_engine
