#pragma once

#include <ostream>

#include <arrow/acero/exec_plan.h>
#include <arrow/io/interfaces.h>
#include <arrow/ipc/writer.h>

#include "silo/query_engine/exec_node/arrow_batch_sink.h"

namespace silo::query_engine::exec_node {

/// Adapter that wraps a std::ostream as an Arrow OutputStream for IPC writing
/// This is required by the arrow::ipc::RecordBatchWriter
class OstreamWrapper : public arrow::io::OutputStream {
   std::ostream* output_stream;
   int64_t position_ = 0;
   bool is_closed_ = false;

  public:
   explicit OstreamWrapper(std::ostream* output_stream);

   arrow::Status Close() override;
   bool closed() const override;
   arrow::Result<int64_t> Tell() const override;
   arrow::Status Write(const void* data, int64_t nbytes) override;
   arrow::Status Flush() override;
};

/// Writer for streaming Arrow IPC format to an ostream.
/// Usage:
///   auto writer = ArrowIpcSink::Make(output_stream, schema);
///   for each batch:
///     writer.writeBatch(batch);
///   writer.finish();
class ArrowIpcSink : public ArrowBatchSink {
   std::shared_ptr<OstreamWrapper> output_wrapper;
   std::shared_ptr<arrow::ipc::RecordBatchWriter> writer;
   std::shared_ptr<arrow::Schema> schema;

   ArrowIpcSink(
      std::shared_ptr<OstreamWrapper> output_wrapper,
      std::shared_ptr<arrow::ipc::RecordBatchWriter> writer,
      std::shared_ptr<arrow::Schema> schema
   )
       : output_wrapper(output_wrapper),
         writer(writer),
         schema(schema) {}

  public:
   static arrow::Result<ArrowIpcSink> Make(
      std::ostream* output_stream,
      std::shared_ptr<arrow::Schema> schema
   );
   arrow::Status writeBatch(const arrow::compute::ExecBatch& batch) override;
   arrow::Status finish() override;
};

}  // namespace silo::query_engine::exec_node
