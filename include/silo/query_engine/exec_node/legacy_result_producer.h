#pragma once

#include <arrow/acero/exec_plan.h>
#include <arrow/record_batch.h>
#include <arrow/builder.h>
#include <spdlog/spdlog.h>

#include "silo/query_engine/query.h"
#include "silo/query_engine/query_result.h"

namespace silo::query_engine::exec_node {

using filter::expressions::Expression;
using filter::operators::Operator;

class JsonValueTypeArrayBuilder{
   std::variant<arrow::Int32Builder, arrow::DoubleBuilder, arrow::StringBuilder, arrow::BooleanBuilder> builder;
  public:
   JsonValueTypeArrayBuilder(std::shared_ptr<arrow::DataType> type);

   arrow::Status insert(const std::optional<std::variant<std::string, bool, int32_t, double>>& value);

   arrow::Datum toDatum() &&;
};

class LegacyResultProducer : public arrow::acero::ExecNode {
   QueryResult query_result;

   std::vector<JsonValueTypeArrayBuilder> arrays;
   std::vector<const std::string*> field_names;

  public:
   LegacyResultProducer(arrow::acero::ExecPlan* plan,
                        std::shared_ptr<arrow::Schema> output_schema,
                        std::shared_ptr<Database> database,
                        std::shared_ptr<Query> query);

   virtual const char* kind_name() const override { return "LegacyResultProducer"; }

   virtual arrow::Status InputReceived(ExecNode* input, arrow::ExecBatch batch) override {
      SILO_PANIC("LegacyResultProducer does not support having inputs.");
   }

   virtual arrow::Status StopProducingImpl() override { SILO_UNIMPLEMENTED(); }

   /// Mark the inputs finished after the given number of batches.
   ///
   /// This may be called before all inputs are received.  This simply fixes
   /// the total number of incoming batches for an input, so that the ExecNode
   /// knows when it has received all input, regardless of order.
   virtual arrow::Status InputFinished(ExecNode* input, int total_batches) override {
      SILO_PANIC("LegacyResultProducer does not support having inputs.");
   }

   void prepareOutputArrays();

   arrow::Status flushOutput();

   arrow::Status produce();

   arrow::Status StartProducing() override;

   arrow::Status StopProducing() override;

   void PauseProducing(arrow::acero::ExecNode* output, int32_t counter) override {}

   void ResumeProducing(arrow::acero::ExecNode* output, int32_t counter) override {}
};

}  // namespace silo::query_engine
