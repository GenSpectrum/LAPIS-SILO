#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/builder.h>
#include <arrow/record_batch.h>
#include <nlohmann/json_fwd.hpp>

#include "rhydb/common/bitmap.h"
#include "rhydb/query_engine/exec_node/arrow_util.h"
#include "rhydb/query_engine/operators/compute_filter.h"
#include "rhydb/storage/table.h"

namespace rhydb::query_engine::exec_node {

class ExecBatchBuilder {
   std::map<schema::ColumnType, std::map<std::string, std::shared_ptr<arrow::ArrayBuilder>>>
      array_builders;
   std::vector<rhydb::schema::ColumnIdentifier> output_fields;

  public:
   explicit ExecBatchBuilder(std::vector<rhydb::schema::ColumnIdentifier> output_fields);

   template <storage::column::Column Column>
   std::map<std::string, ArrowBuilder<Column>*> getColumnTypeArrayBuilders() {
      std::map<std::string, ArrowBuilder<Column>*> result;
      for (auto& [builder_name, builder] : array_builders.at(Column::TYPE)) {
         result.emplace(builder_name, dynamic_cast<ArrowBuilder<Column>*>(builder.get()));
      }
      return result;
   }

   arrow::Status appendEntries(const storage::Table& table, const Bitmap& row_ids);

   arrow::Result<arrow::ExecBatch> finishBatch(int64_t length);
};

/// Adds a source node to `plan` that produces `columns` of the rows of `table` matched by
/// `filter`. The filter is evaluated when the plan starts, and the rows are produced in batches
/// limited by `batch_size_cutoff`, one batch per task, pausing while the output applies
/// backpressure.
arrow::Result<arrow::acero::ExecNode*> makeTableScan(
   arrow::acero::ExecPlan* plan,
   const std::vector<rhydb::schema::ColumnIdentifier>& columns,
   operators::CompiledFilter filter,
   std::shared_ptr<const storage::Table> table,
   size_t batch_size_cutoff
);

}  // namespace rhydb::query_engine::exec_node
