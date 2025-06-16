#include <memory>
#include <string>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/builder.h>
#include <arrow/record_batch.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/column/column_type_visitor.h"
#include "silo/storage/table.h"

namespace silo::query_engine::exec_node {

class TableScan : public arrow::acero::ExecNode {
   std::map<schema::ColumnType, std::map<std::string, std::unique_ptr<arrow::ArrayBuilder>>>
      array_builders;

   std::shared_ptr<std::vector<std::unique_ptr<filter::operators::Operator>>>
      partition_filter_operators;

   std::vector<silo::schema::ColumnIdentifier> output_fields;
   const std::shared_ptr<const storage::Table> table;
   size_t batch_size_cutoff;

   // We need to tell the consumer how many batches we produced in total
   size_t num_batches_produced = 0;

  public:
   TableScan(
      arrow::acero::ExecPlan* plan,
      const std::vector<silo::schema::ColumnIdentifier>& columns,
      std::shared_ptr<std::vector<std::unique_ptr<filter::operators::Operator>>>
         partition_filter_operators,
      std::shared_ptr<const storage::Table> table,
      size_t batch_size_cutoff
   )
       : arrow::acero::ExecNode(plan, {}, {}, columnsToInternalArrowSchema(columns)),
         partition_filter_operators(partition_filter_operators),
         output_fields(columns),
         table(table),
         batch_size_cutoff(batch_size_cutoff) {
      prepareOutputArrays();
   }

   template <storage::column::Column Column>
   std::map<std::string, ArrowBuilder<Column>*> getColumnTypeArrayBuilders() {
      std::map<std::string, ArrowBuilder<Column>*> result;
      for (auto& [builder_name, builder] : array_builders.at(Column::TYPE)) {
         result.emplace(builder_name, dynamic_cast<ArrowBuilder<Column>*>(builder.get()));
      }
      return result;
   }

  private:
   void prepareOutputArrays();

   const arrow::Ordering& ordering() const override { return arrow::Ordering::Implicit(); }

   const char* kind_name() const override { return "TableScan"; }

   arrow::Status InputReceived(ExecNode* input, arrow::ExecBatch batch) override {
      SILO_PANIC("TableScan does not support having inputs.");
   }

   arrow::Status InputFinished(ExecNode* input, int total_batches) override {
      SILO_PANIC("TableScan does not support having inputs.");
   }

   arrow::Status StartProducing() override {
      SPDLOG_TRACE("TableScan::StartProducing");
      try {
         ARROW_RETURN_NOT_OK(produce());
         return arrow::Status::OK();
      } catch (const std::exception& exception) {
         return arrow::Status::ExecutionError(exception.what());
      }
   }

   arrow::Status StopProducingImpl() override {
      SPDLOG_TRACE("TableScan::StopProducing");
      return arrow::Status::OK();
   }

   void PauseProducing(arrow::acero::ExecNode* output, int32_t counter) override {}

   void ResumeProducing(arrow::acero::ExecNode* output, int32_t counter) override {}

  private:
   arrow::Status flushOutput();

   arrow::Status produce();

   arrow::Status appendEntries(
      const storage::TablePartition& table_partition,
      const roaring::Roaring& row_ids
   );
};

}  // namespace silo::query_engine::exec_node