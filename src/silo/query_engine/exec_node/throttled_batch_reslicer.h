#pragma once

#include <chrono>
#include <utility>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/query_context.h>
#include <arrow/compute/ordering.h>
#include <arrow/util/async_generator_fwd.h>
#include <spdlog/spdlog.h>

#include "silo/common/panic.h"

namespace silo::query_engine::exec_node {

using arrow::acero::BackpressureMonitor;

class ThrottledBatchReslicer {
   arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> input_batches;
   int64_t batch_size;
   std::chrono::milliseconds target_batch_rate;
   BackpressureMonitor* backpressure_monitor;

   std::optional<arrow::ExecBatch> current_batch;
   int64_t offset;
   int64_t remaining;  // always >0 when current_batch != std::nullopt

   std::optional<std::chrono::system_clock::time_point> last_batch_delivered;

  public:
   ThrottledBatchReslicer(
      arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> input_batches,
      int64_t batch_size,
      std::chrono::milliseconds target_batch_rate,
      BackpressureMonitor* backpressure_monitor
   )
       : input_batches(std::move(input_batches)),
         batch_size(batch_size),
         target_batch_rate(target_batch_rate),
         backpressure_monitor(backpressure_monitor) {
      SILO_ASSERT(batch_size > 0);
   }

   // By arrow specification, this function will not be called re-entrantly
   arrow::Future<std::optional<arrow::compute::ExecBatch>> operator()();

  private:
   void delayForTargetBatchRate();

   arrow::Result<std::optional<arrow::ExecBatch>> deliverSlicedBatch();
};

}  // namespace silo::query_engine::exec_node