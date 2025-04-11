#pragma once

#include <arrow/acero/exec_plan.h>
#include <arrow/record_batch.h>

#include "silo/query_engine/query_result.h"

namespace silo::query_engine {

using filter::expressions::Expression;
using filter::operators::Operator;

QueryResult createLegacyQueryResult(const Query& query, const Database& database) {
   SPDLOG_DEBUG("Parsed query: {}", query.filter->toString());

   std::vector<std::string> compiled_queries(database.table.getNumberOfPartitions());
   std::vector<CopyOnWriteBitmap> partition_filters(database.table.getNumberOfPartitions());
   for (size_t partition_index = 0; partition_index != database.table.getNumberOfPartitions();
        partition_index++) {
      std::unique_ptr<Operator> part_filter = query.filter->compile(
         database, database.table.getPartition(partition_index), Expression::AmbiguityMode::NONE
      );
      compiled_queries[partition_index] = part_filter->toString();
      partition_filters[partition_index] = part_filter->evaluate();
   }

   for (uint32_t i = 0; i < database.table.getNumberOfPartitions(); ++i) {
      SPDLOG_DEBUG("Simplified query for partition {}: {}", i, compiled_queries[i]);
   }

   return query.action->executeAndOrder(database, std::move(partition_filters));
}

class LegacyResultProducerOptions : public arrow::acero::ExecNodeOptions {
  public:
   std::shared_ptr<arrow::Schema> output_schema;
   std::shared_ptr<Database> database;
   const Query& query;

   LegacyResultProducerOptions(
      std::shared_ptr<arrow::Schema> output_schema,
      std::shared_ptr<Database> database,
      const Query& query
   )
       : output_schema(output_schema),
         database(database),
         query(query) {}
};

class LegacyResultProducer : public arrow::acero::ExecNode {
   QueryResult query_result;

   std::atomic<bool> running = true;
   std::thread producer_thread;

  public:
   LegacyResultProducer(arrow::acero::ExecPlan* plan, const LegacyResultProducerOptions& options)
       : arrow::acero::ExecNode(plan, {}, {}, options.output_schema) {
      query_result = createLegacyQueryResult(options.query, *options.database);
   }

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

   arrow::Status produce() {
      if (running) {
         auto x = query_result.entries();
         std::shared_ptr<arrow::RecordBatch> batch;
         ARROW_ASSIGN_OR_RAISE(batch, arrow::RecordBatch::MakeEmpty(output_schema_));
         auto batch_datum = arrow::Datum{batch};
         arrow::ExecBatch exec_batch;
         ARROW_ASSIGN_OR_RAISE(exec_batch, arrow::compute::ExecBatch::Make({batch_datum}, -1));
         ARROW_RETURN_NOT_OK(this->output_->InputReceived(this, exec_batch));
      }
      return arrow::Status::OK();
   }

   arrow::Status StartProducing() override {
      running.store(true);
      producer_thread = std::thread([this]() {
         arrow::Status status = this->produce();
         if (!status.ok()) {
            // Handle error or propagate
         }
      });
      return arrow::Status::OK();
   }

   arrow::Status StopProducing() override {
      running.store(false);
      if (producer_thread.joinable()) {
         producer_thread.join();
      }
      return arrow::Status::OK();
   }

   void PauseProducing(arrow::acero::ExecNode* output, int32_t counter) override {}

   void ResumeProducing(arrow::acero::ExecNode* output, int32_t counter) override {}
};

}  // namespace silo::query_engine
