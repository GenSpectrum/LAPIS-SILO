#include "silo/query_engine/exec_node/arrow_ipc_sink.h"

#include <spdlog/spdlog.h>

#include "evobench/evobench.hpp"

namespace silo::query_engine::exec_node {

OstreamWrapper::OstreamWrapper(std::ostream* output_stream)
    : output_stream(output_stream) {}

arrow::Status OstreamWrapper::Close() {
   is_closed_ = true;
   output_stream->flush();
   if (!*output_stream) {
      return arrow::Status::IOError("Failed to flush output stream on close");
   }
   return arrow::Status::OK();
}

bool OstreamWrapper::closed() const {
   return is_closed_;
}

arrow::Result<int64_t> OstreamWrapper::Tell() const {
   return position_;
}

arrow::Status OstreamWrapper::Write(const void* data, int64_t nbytes) {
   output_stream->write(static_cast<const char*>(data), static_cast<std::streamsize>(nbytes));
   if (!*output_stream) {
      return arrow::Status::IOError("Failed to write to output stream");
   }
   position_ += nbytes;
   return arrow::Status::OK();
}

arrow::Status OstreamWrapper::Flush() {
   output_stream->flush();
   if (!*output_stream) {
      return arrow::Status::IOError("Failed to flush output stream");
   }
   return arrow::Status::OK();
}

arrow::Result<ArrowIpcSink> ArrowIpcSink::Make(
   std::ostream* output_stream,
   std::shared_ptr<arrow::Schema> schema
) {
   auto output_wrapper = std::make_shared<OstreamWrapper>(output_stream);
   ARROW_ASSIGN_OR_RAISE(auto writer, arrow::ipc::MakeStreamWriter(output_wrapper, schema));
   return ArrowIpcSink(output_wrapper, writer, schema);
}

arrow::Status ArrowIpcSink::writeBatch(const arrow::compute::ExecBatch& batch) {
   EVOBENCH_SCOPE("QueryPlan", "writeBatchAsArrowIpc");

   ARROW_ASSIGN_OR_RAISE(auto record_batch, batch.ToRecordBatch(schema));
   ARROW_RETURN_NOT_OK(writer->WriteRecordBatch(*record_batch));

   return arrow::Status::OK();
}

arrow::Status ArrowIpcSink::finish() {
   return writer->Close();
}

}  // namespace silo::query_engine::exec_node
