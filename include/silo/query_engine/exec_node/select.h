#include <memory>
#include <string>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/builder.h>
#include <arrow/record_batch.h>
#include <nlohmann/json_fwd.hpp>

#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/column/column_type_visitor.h"

namespace silo::query_engine::exec_node {

// TODO change to TableScan
class Select : public arrow::acero::ExecNode {
  public:
   // TODO change from X Pattern to variant struct
#define X(Column, ColumnType, name) std::map<std::string, ArrowBuilder<Column>> name##_arrays;
#include "silo/storage/column/all_column_names.h"
#undef X

   std::vector<CopyOnWriteBitmap> partition_filters;

   std::vector<silo::schema::ColumnIdentifier> output_fields;

   const std::shared_ptr<const storage::Table> table;

  public:
   Select(
      arrow::acero::ExecPlan* plan,
      const std::vector<silo::schema::ColumnIdentifier>& columns,
      const std::vector<std::unique_ptr<filter::operators::Operator>>& partition_filter_operators,
      std::shared_ptr<const storage::Table> table,

   )
       : arrow::acero::ExecNode(plan, {}, {}, columnsToArrowSchema(columns)),
         output_fields(columns),
         table(table) {
      for (const auto& partition_filter_operator : partition_filter_operators) {
         partition_filters.emplace_back(partition_filter_operator->evaluate());
      }
      prepareOutputArrays();
   }

  public:
   template <storage::column::Column ActualColumn>
   std::map<std::string, ArrowBuilder<ActualColumn>>& getColumnTypeArrayBuilders() {
// TODO think whether we actually need the X-pattern
#define X(Column, ColumnType, name)                      \
   if constexpr (std::is_same<Column, ActualColumn>()) { \
      return name##_arrays;                              \
   } else
#include "silo/storage/column/all_column_names.h"
#undef X
      { SILO_UNREACHABLE(); }
   }

  private:
   void prepareOutputArrays();

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

   arrow::Status StartProducing() override { return produce(); }

   arrow::Status StopProducing() override { return arrow::Status::OK(); }

   void PauseProducing(arrow::acero::ExecNode* output, int32_t counter) override {}

   void ResumeProducing(arrow::acero::ExecNode* output, int32_t counter) override {}

   arrow::Status flushOutput();

   static constexpr size_t MATERIALIZATION_CUTOFF = 50000;

   arrow::Status produce();

   arrow::Status appendEntries(
      const storage::TablePartition& table_partition,
      const roaring::Roaring& row_ids
   );
};

}  // namespace silo::query_engine::exec_node