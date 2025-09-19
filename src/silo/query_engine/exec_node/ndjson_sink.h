#pragma once

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/acero/query_context.h>
#include <arrow/util/async_generator_fwd.h>
#include <spdlog/spdlog.h>

#include "silo/common/panic.h"

namespace silo::query_engine::exec_node {

arrow::Status writeBatchAsNdjson(
   arrow::compute::ExecBatch batch,
   const std::shared_ptr<arrow::Schema>& schema,
   std::ostream* output_stream
);

arrow::Result<arrow::acero::BackpressureMonitor*> createGenerator(
   arrow::acero::ExecPlan* plan,
   arrow::acero::ExecNode* input,
   arrow::AsyncGenerator<std::optional<arrow::ExecBatch>>* generator
);

}  // namespace silo::query_engine::exec_node
