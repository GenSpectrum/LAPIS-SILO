#pragma once

#include <arrow/acero/exec_plan.h>
#include <arrow/builder.h>
#include <arrow/record_batch.h>
#include <spdlog/spdlog.h>

#include "silo/query_engine/exec_node/json_value_type_array_builder.h"
#include "silo/query_engine/query.h"
#include "silo/query_engine/query_result.h"

namespace silo::query_engine::exec_node {

using filter::expressions::Expression;
using filter::operators::Operator;

class LegacyResultProducer : public arrow::acero::ExecNode {
   QueryResult query_result;

   size_t materialization_cutoff;

   std::vector<JsonValueTypeArrayBuilder> array_builders;
   std::vector<const std::string*> field_names;

   size_t num_batches_produced = 0;

  public:
   LegacyResultProducer(
      arrow::acero::ExecPlan* plan,
      const std::vector<silo::schema::ColumnIdentifier>& columns,
      std::shared_ptr<const storage::Table> table,
      std::shared_ptr<filter::operators::OperatorVector> partition_filter_operators,
      const actions::Action* action,
      size_t materialization_cutoff
   );

   const arrow::Ordering& ordering() const override { return arrow::Ordering::Implicit(); }

   const char* kind_name() const override { return "LegacyResultProducer"; }

   arrow::Status InputReceived(ExecNode* input, arrow::ExecBatch batch) override {
      SILO_PANIC("LegacyResultProducer does not support having inputs.");
   }

   arrow::Status InputFinished(ExecNode* input, int total_batches) override {
      SILO_PANIC("LegacyResultProducer does not support having inputs.");
   }

   arrow::Status StartProducing() override {
      try {
         SPDLOG_TRACE("LegacyResultProducer::StartProducing");
         ARROW_RETURN_NOT_OK(produce());
         return output_->InputFinished(this, num_batches_produced);
      } catch (const std::exception& error) {
         SPDLOG_ERROR("TableScan::produce exited with error: {}", error.what());
         return arrow::Status::ExecutionError(error.what());
      }
   }

   arrow::Status StopProducingImpl() override { return arrow::Status::OK(); }

   void PauseProducing(arrow::acero::ExecNode* output, int32_t counter) override {}

   void ResumeProducing(arrow::acero::ExecNode* output, int32_t counter) override {}

  private:
   void prepareOutputArrays();

   arrow::Status flushOutput();

   arrow::Status produce();
};

}  // namespace silo::query_engine::exec_node
