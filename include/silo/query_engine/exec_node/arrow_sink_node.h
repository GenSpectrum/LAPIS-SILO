#pragma once

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/ipc/writer.h>

namespace silo::query_engine::exec_node {
class ArrowOutputStreamWrapper : public arrow::io::OutputStream {
   std::ostream* output;
   bool is_closed = false;

  public:
   ArrowOutputStreamWrapper(std::ostream* output)
       : output(output) {}

   arrow::Status Close() override {
      is_closed = true;
      output->flush();
      output = nullptr;
      return arrow::Status::OK();
   }

   arrow::Status Write(const void* data, int64_t nbytes) override {
      if (is_closed) {
         return arrow::Status::Invalid("Output is already closed");
      }
      auto& x = output->write(reinterpret_cast<const char*>(data), nbytes);
      if (x.good()) {
         return arrow::Status::OK();
      }
      return arrow::Status::IOError("Failed to write to output stream: ", x.badbit);
   }

   arrow::Result<int64_t> Tell() const override {
      if (is_closed) {
         return arrow::Status::Invalid("Output is already closed");
      }
      return output->tellp();
   }

   bool closed() const override { return is_closed; }
};

class ArrowSinkNode : public arrow::acero::ExecNode {
   ArrowOutputStreamWrapper output_stream;
   std::shared_ptr<arrow::ipc::RecordBatchWriter> writer;
   int batches_written = 0;
   std::optional<int> total_batches_from_input = std::nullopt;

  public:
   ArrowSinkNode(arrow::acero::ExecPlan* plan, std::ostream* stream, arrow::acero::ExecNode* input)
       : arrow::acero::ExecNode(plan, {input}, {"input"}, nullptr),
         output_stream(stream),
         writer(arrow::ipc::MakeStreamWriter(&output_stream, input->output_schema()).ValueOrDie()) {
   }

   const char* kind_name() const override { return "ArrowSinkNode"; };

   arrow::Status InputReceived(arrow::acero::ExecNode* input, arrow::compute::ExecBatch batch)
      override {
      std::shared_ptr<arrow::RecordBatch> record_batch;
      ARROW_ASSIGN_OR_RAISE(record_batch, batch.ToRecordBatch(input->output_schema()));
      ARROW_RETURN_NOT_OK(writer->WriteRecordBatch(*record_batch));
      batches_written += 1;
      if (batches_written == total_batches_from_input) {
         ARROW_RETURN_NOT_OK(writer->Close());
      }
      return arrow::Status::OK();
   }

   arrow::Status InputFinished(arrow::acero::ExecNode* input, int total_batches) override {
      this->total_batches_from_input = total_batches;
      if (total_batches == 0) {
         ARROW_RETURN_NOT_OK(writer->Close());
      }
      return arrow::Status::OK();
   }

   arrow::Status StartProducing() override { return arrow::Status::OK(); }
   arrow::Status StopProducing() override { return arrow::Status::OK(); }
   arrow::Status StopProducingImpl() override { return arrow::Status::OK(); }

   void ResumeProducing(arrow::acero::ExecNode* output, int32_t counter) override {}

   void PauseProducing(arrow::acero::ExecNode* output, int32_t counter) override {}
};

}  // namespace silo::query_engine::exec_node