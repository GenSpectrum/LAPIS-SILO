#include "rhydb/query_engine/exec_node/async_batch_producer.h"

#include <exception>
#include <memory>
#include <utility>

#include <arrow/acero/query_context.h>
#include <arrow/util/thread_pool.h>

namespace rhydb::query_engine::exec_node {

arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> makeAsyncBatchGenerator(
   arrow::acero::ExecPlan& plan,
   BatchProducer produce_next_batch
) {
   arrow::internal::Executor* executor = plan.query_context()->executor();
   // Shared, because the source node copies its generator, which would copy the producer's state
   auto producer = std::make_shared<BatchProducer>(std::move(produce_next_batch));
   auto produce = [producer] -> arrow::Result<std::optional<arrow::ExecBatch>> {
      try {
         return (*producer)();
      } catch (const std::exception& exception) {
         return arrow::Status::ExecutionError(exception.what());
      }
   };
#ifdef __EMSCRIPTEN__
   // The browser build runs acero with a single-threaded executor, so we produce inline.
   (void)executor;
   return [produce] { return arrow::Future<std::optional<arrow::ExecBatch>>{produce()}; };
#else
   return [executor, produce] { return arrow::DeferNotOk(executor->Submit(produce)); };
#endif
}

}  // namespace rhydb::query_engine::exec_node
