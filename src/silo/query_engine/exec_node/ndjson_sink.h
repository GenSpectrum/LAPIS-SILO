#pragma once

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/acero/query_context.h>
#include <arrow/util/async_generator_fwd.h>
#include <spdlog/spdlog.h>

#include "silo/query_engine/exec_node/arrow_batch_sink.h"

namespace silo::query_engine::exec_node {

class NdjsonSink : public ArrowBatchSink {
   std::ostream* output_stream;
   std::shared_ptr<arrow::Schema> schema;

  public:
   NdjsonSink(std::ostream* output_stream_, std::shared_ptr<arrow::Schema> schema_)
       : output_stream(output_stream_),
         schema(std::move(schema_)) {}

   arrow::Status writeBatch(const arrow::compute::ExecBatch& batch) override;
   arrow::Status finish() override;
};

}  // namespace silo::query_engine::exec_node
