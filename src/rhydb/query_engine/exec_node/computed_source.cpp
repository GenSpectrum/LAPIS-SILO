#include "rhydb/query_engine/exec_node/computed_source.h"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <memory>
#include <utility>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/query_context.h>
#include <arrow/compute/exec.h>
#include <arrow/compute/ordering.h>
#include <arrow/result.h>
#include <arrow/status.h>

#include "rhydb/common/panic.h"

namespace rhydb::query_engine::exec_node {

ComputedSourceNode::ComputedSourceNode(
   arrow::acero::ExecPlan* plan,
   std::shared_ptr<arrow::Schema> output_schema
)
    : ExecNode(plan, {}, {}, std::move(output_schema)) {}

const arrow::Ordering& ComputedSourceNode::ordering() const {
   static const arrow::Ordering implicit = arrow::Ordering::Implicit();
   return implicit;
}

arrow::Status ComputedSourceNode::InputReceived(
   arrow::acero::ExecNode* /*input*/,
   arrow::ExecBatch /*batch*/
) {
   RHYDB_UNREACHABLE();
}

arrow::Status ComputedSourceNode::InputFinished(
   arrow::acero::ExecNode* /*input*/,
   int /*total_batches*/
) {
   RHYDB_UNREACHABLE();
}

arrow::Status ComputedSourceNode::StartProducing() {
   plan_->query_context()->ScheduleTask(
      [this] -> arrow::Status {
         try {
            return computeAndEmit();
         } catch (const std::exception& exception) {
            return arrow::Status::ExecutionError(exception.what());
         }
      },
      kind_name()
   );
   return arrow::Status::OK();
}

// The output is computed in one go, so there is nothing to pause
void ComputedSourceNode::PauseProducing(arrow::acero::ExecNode* /*output*/, int32_t /*counter*/) {}

void ComputedSourceNode::ResumeProducing(arrow::acero::ExecNode* /*output*/, int32_t /*counter*/) {}

arrow::Status ComputedSourceNode::StopProducingImpl() {
   stopped_ = true;
   return arrow::Status::OK();
}

arrow::Status ComputedSourceNode::computeAndEmit() {
   if (stopped_) {
      return arrow::Status::OK();
   }
   ARROW_ASSIGN_OR_RAISE(const arrow::ExecBatch result, compute());

   constexpr int64_t MAX_BATCH_SIZE = arrow::acero::ExecPlan::kMaxBatchSize;
   int batch_index = 0;
   for (int64_t offset = 0; offset < result.length && !stopped_; offset += MAX_BATCH_SIZE) {
      arrow::ExecBatch slice =
         result.Slice(offset, std::min(MAX_BATCH_SIZE, result.length - offset));
      slice.index = batch_index++;
      ARROW_RETURN_NOT_OK(output_->InputReceived(this, std::move(slice)));
   }
   return output_->InputFinished(this, batch_index);
}

}  // namespace rhydb::query_engine::exec_node
