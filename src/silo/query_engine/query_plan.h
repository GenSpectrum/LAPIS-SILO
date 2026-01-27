#pragma once

#include <optional>
#include <ostream>
#include <utility>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/util/async_generator_fwd.h>

#include "silo/query_engine/exec_node/produce_guard.h"

namespace silo::query_engine {

class QueryPlan {
  public:
   std::shared_ptr<arrow::acero::ExecPlan> arrow_plan;
   std::shared_ptr<arrow::Schema> results_schema;
   // The function which returns the exec batches in the correct order of the output
   arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> results_generator;
   arrow::acero::BackpressureMonitor* backpressure_monitor;
   std::string_view request_id;

   // This guard will prevent any TableScans from producing batches until MarkFinished is called on
   // this future.
   // This guards against an arrow bug where producing batches too quickly at start-up can
   // (i) lead to errors and (ii) clog up the plan with batches before back-pressure can kick in
   // https://github.com/apache/arrow/issues/47641 https://github.com/apache/arrow/issues/47642
   std::unique_ptr<exec_node::ProduceGuard> produce_guard;

   static arrow::Result<QueryPlan> makeQueryPlan(
      std::shared_ptr<arrow::acero::ExecPlan> arrow_plan,
      arrow::acero::ExecNode* root,
      std::string_view request_id,
      std::unique_ptr<exec_node::ProduceGuard> produce_guard
   );

   static arrow::Result<QueryPlan> makeQueryPlan(
      std::shared_ptr<arrow::acero::ExecPlan> arrow_plan,
      arrow::acero::ExecNode* root,
      std::string_view request_id
   );

   void executeAndWrite(std::ostream* output_stream, uint64_t timeout_in_seconds);

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

   arrow::Status executeAndWriteImpl(std::ostream* output_stream, uint64_t timeout_in_seconds);
};

}  // namespace silo::query_engine
