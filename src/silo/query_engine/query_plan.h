#pragma once

#include <optional>
#include <ostream>
#include <utility>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/util/async_generator_fwd.h>

#include "silo/query_engine/exec_node/arrow_batch_sink.h"

namespace silo::query_engine {

class QueryPlan {
  public:
   std::shared_ptr<arrow::acero::ExecPlan> arrow_plan;
   std::shared_ptr<arrow::Schema> results_schema;
   // The function which returns the exec batches in the correct order of the output
   arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> results_generator;
   arrow::acero::BackpressureMonitor* backpressure_monitor;
   std::string_view request_id;

   static arrow::Result<QueryPlan> makeQueryPlan(
      std::shared_ptr<arrow::acero::ExecPlan> arrow_plan,
      arrow::acero::ExecNode* root,
      std::string_view request_id
   );

   void executeAndWrite(exec_node::ArrowBatchSink& output_sink, uint64_t timeout_in_seconds);

  private:
   QueryPlan(
      std::shared_ptr<arrow::acero::ExecPlan> arrow_plan,
      arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> results_generator,
      arrow::acero::BackpressureMonitor* backpressure_monitor,
      std::string_view request_id
   )
       : arrow_plan(std::move(arrow_plan)),
         results_generator(std::move(results_generator)),
         backpressure_monitor(backpressure_monitor),
         request_id(request_id) {}

   arrow::Status executeAndWriteImpl(
      exec_node::ArrowBatchSink& output_sink,
      uint64_t timeout_in_seconds
   );

   static arrow::Result<arrow::acero::BackpressureMonitor*> createGenerator(
      arrow::acero::ExecPlan* plan,
      arrow::acero::ExecNode* input,
      arrow::AsyncGenerator<std::optional<arrow::ExecBatch>>* generator
   );
};

}  // namespace silo::query_engine
