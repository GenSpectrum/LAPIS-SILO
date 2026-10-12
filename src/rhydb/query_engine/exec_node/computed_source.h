#pragma once

#include <atomic>
#include <memory>
#include <string>

#include <arrow/acero/exec_plan.h>
#include <arrow/compute/exec.h>
#include <arrow/compute/ordering.h>
#include <arrow/result.h>
#include <arrow/status.h>
#include <arrow/type_fwd.h>

namespace rhydb::query_engine::exec_node {

/// A source node without inputs that computes its whole output at once.
///
/// `StartProducing` only schedules `compute()` as a task on the plan's executor, so independent
/// sources of a plan compute concurrently. The result is emitted in slices of at most
/// `ExecPlan::kMaxBatchSize` rows.
class ComputedSourceNode : public arrow::acero::ExecNode {
  public:
   ComputedSourceNode(arrow::acero::ExecPlan* plan, std::shared_ptr<arrow::Schema> output_schema);

   [[nodiscard]] const arrow::Ordering& ordering() const override;

   [[noreturn]] arrow::Status InputReceived(
      arrow::acero::ExecNode* input,
      arrow::ExecBatch batch
   ) final;
   [[noreturn]] arrow::Status InputFinished(arrow::acero::ExecNode* input, int total_batches) final;

   arrow::Status StartProducing() final;
   void PauseProducing(arrow::acero::ExecNode* output, int32_t counter) final;
   void ResumeProducing(arrow::acero::ExecNode* output, int32_t counter) final;

  protected:
   arrow::Status StopProducingImpl() final;

  private:
   virtual arrow::Result<arrow::ExecBatch> compute() = 0;

   arrow::Status computeAndEmit();

   std::atomic<bool> stopped_ = false;
};

}  // namespace rhydb::query_engine::exec_node
