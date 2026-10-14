#pragma once

#include <functional>
#include <optional>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/result.h>
#include <arrow/util/async_generator_fwd.h>
#include <arrow/util/future.h>

namespace rhydb::query_engine::exec_node {

/// Synchronously produces the next batch, or `std::nullopt` at the end of the stream
using BatchProducer = std::function<arrow::Result<std::optional<arrow::ExecBatch>>()>;

/// Turns a synchronous producer into a source node generator, which produces each batch as a task
/// on the CPU executor of `plan`.
///
/// An acero source node pulls its first batch inside `StartProducing()`, which the plan calls for
/// each source in turn on the thread that starts the plan. A source that computes its batch inline
/// therefore runs strictly after the previously started one, e.g. the inputs of a `unionall` are
/// evaluated one after another instead of in parallel. Producing inline also lets a source run
/// ahead of backpressure (https://github.com/apache/arrow/issues/47641) and hangs the plan if the
/// producer throws (https://github.com/apache/arrow/issues/47642).
///
/// By acero specification, the generator is not called re-entrantly, so the producer may keep
/// state across calls.
arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> makeAsyncBatchGenerator(
   arrow::acero::ExecPlan& plan,
   BatchProducer produce_next_batch
);

}  // namespace rhydb::query_engine::exec_node
