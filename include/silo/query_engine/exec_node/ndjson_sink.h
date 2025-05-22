#pragma once

#include <arrow/acero/exec_plan.h>

namespace silo::query_engine::exec_node {

class NdjsonSink : public arrow::acero::ExecNode {
   std::ostream* output_stream;
   std::atomic<int> batches_written = 0;
   std::atomic<int> total_batches_from_input = -1;

  public:
   NdjsonSink(arrow::acero::ExecPlan* plan, std::ostream* stream, arrow::acero::ExecNode* input)
       : arrow::acero::ExecNode(plan, {input}, {"input"}, nullptr),
         output_stream(stream) {}

   const char* kind_name() const override { return "NdjsonSinkNode"; }

   arrow::Status writeRecordBatchAsNdjson(std::shared_ptr<arrow::RecordBatch> record_batch);

   arrow::Status InputReceived(arrow::acero::ExecNode* input, arrow::compute::ExecBatch batch)
      override;

   arrow::Status InputFinished(arrow::acero::ExecNode* input, int total_batches) override;

   arrow::Status StartProducing() override;

   arrow::Status StopProducing() override;
   arrow::Status StopProducingImpl() override { return arrow::Status::OK(); }

   void ResumeProducing(arrow::acero::ExecNode* output, int32_t counter) override {}

   void PauseProducing(arrow::acero::ExecNode* output, int32_t counter) override {}
};

}  // namespace silo::query_engine::exec_node
