#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/builder.h>
#include <arrow/record_batch.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/batched_bitmap_reader.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/storage/table.h"

namespace silo::query_engine::exec_node {

class ExecBatchBuilder {
   std::map<schema::ColumnType, std::map<std::string, std::shared_ptr<arrow::ArrayBuilder>>>
      array_builders;
   std::vector<silo::schema::ColumnIdentifier> output_fields;

  public:
   explicit ExecBatchBuilder(std::vector<silo::schema::ColumnIdentifier> output_fields);

   template <storage::column::Column Column>
   std::map<std::string, ArrowBuilder<Column>*> getColumnTypeArrayBuilders() {
      std::map<std::string, ArrowBuilder<Column>*> result;
      for (auto& [builder_name, builder] : array_builders.at(Column::TYPE)) {
         result.emplace(builder_name, dynamic_cast<ArrowBuilder<Column>*>(builder.get()));
      }
      return result;
   }

   arrow::Status appendEntries(
      const storage::TablePartition& table_partition,
      const roaring::Roaring& row_ids
   );

   arrow::Result<arrow::ExecBatch> finishBatch();
};

class TableScanGenerator {
   ExecBatchBuilder exec_batch_builder;

   std::vector<CopyOnWriteBitmap> partition_filters;

   std::optional<BatchedBitmapReader> current_bitmap_reader;
   size_t current_partition_idx;

   const std::shared_ptr<const storage::Table> table;
   size_t batch_size_cutoff;

  public:
   TableScanGenerator(
      const std::vector<silo::schema::ColumnIdentifier>& columns,
      std::vector<CopyOnWriteBitmap> partition_filters_,
      std::shared_ptr<const storage::Table> table,
      size_t batch_size_cutoff
   )
       : exec_batch_builder(columns),
         partition_filters(std::move(partition_filters_)),
         table(std::move(table)),
         batch_size_cutoff(batch_size_cutoff) {
      current_partition_idx = 0;
      if (!partition_filters.empty()) {
         current_bitmap_reader =
            BatchedBitmapReader{partition_filters.front().getConstReference(), batch_size_cutoff};
      }
   }

   arrow::Future<std::optional<arrow::ExecBatch>> operator()() {
      SPDLOG_TRACE("TableScanGenerator::operator()");
      auto future = arrow::Future<std::optional<arrow::ExecBatch>>::Make();
      // We do this to guard against https://github.com/apache/arrow/issues/47641
      // and https://github.com/apache/arrow/issues/47642
      std::thread([future, this]() mutable {
         try {
            auto result = produceNextBatch();
            future.MarkFinished(std::move(result));
         } catch (const std::exception& exception) {
            future.MarkFinished(arrow::Status::ExecutionError(exception.what()));
         }
      }).detach();
      return future;
   };

  private:
   arrow::Result<std::optional<arrow::ExecBatch>> produceNextBatch();
};

arrow::Result<arrow::acero::ExecNode*> makeTableScan(
   arrow::acero::ExecPlan* plan,
   const std::vector<silo::schema::ColumnIdentifier>& columns,
   std::vector<CopyOnWriteBitmap> partition_filters_,
   std::shared_ptr<const storage::Table> table,
   size_t batch_size_cutoff
);

}  // namespace silo::query_engine::exec_node