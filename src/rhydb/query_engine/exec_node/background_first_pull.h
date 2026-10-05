#pragma once

#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <utility>

#include <arrow/acero/exec_plan.h>
#include <arrow/util/future.h>
#include <arrow/util/thread_pool.h>

namespace rhydb::query_engine::exec_node {

using SourceGenerator = std::function<arrow::Future<std::optional<arrow::ExecBatch>>()>;

/// Wraps the generator of an Acero source node so that its first pull runs on the CPU thread pool.
///
/// Acero pulls the first batch of every source node synchronously within
/// `ExecPlan::StartProducing`, one source after the other. A generator that does all of its work
/// in the first pull (e.g. evaluating its filter) would therefore make independent sources, like
/// the inputs of a `unionall`, run sequentially. Moving that pull to the thread pool lets them run
/// concurrently. Later pulls happen on the plan's executor already and run directly.
///
/// The wrapped generator is expected to produce its batches synchronously.
inline SourceGenerator pullFirstBatchInBackground(SourceGenerator generator) {
#ifdef __EMSCRIPTEN__
   // The browser build runs acero with a single-threaded executor, see `TableScanGenerator`
   return generator;
#else
   // Shared with the background task, which may outlive the plan if the plan is aborted
   auto shared_generator = std::make_shared<SourceGenerator>(std::move(generator));
   return [shared_generator,
           first_pull = true] mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (!first_pull) {
         return (*shared_generator)();
      }
      first_pull = false;
      // Acero waits for the returned future before pulling again, so the generator is never
      // invoked concurrently
      return arrow::DeferNotOk(
         arrow::internal::GetCpuThreadPool()->Submit(
            [shared_generator] -> arrow::Result<std::optional<arrow::ExecBatch>> {
               try {
                  return (*shared_generator)().result();
               } catch (const std::exception& exception) {
                  return arrow::Status::ExecutionError(exception.what());
               }
            }
         )
      );
   };
#endif
}

}  // namespace rhydb::query_engine::exec_node
