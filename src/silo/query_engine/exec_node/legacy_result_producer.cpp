#include "silo/query_engine/exec_node/legacy_result_producer.h"

#include "silo/query_engine/exec_node/arrow_util.h"

namespace silo::query_engine::exec_node {

namespace {

using filter::expressions::Expression;
using filter::operators::Operator;

QueryResult createLegacyQueryResult(
   const std::vector<std::unique_ptr<filter::operators::Operator>>& partition_filter_operators,
   const actions::Action* action,
   std::shared_ptr<const storage::Table> table
) {
   std::vector<CopyOnWriteBitmap> partition_filters;
   partition_filters.reserve(table->getNumberOfPartitions());
   for (const auto& partition_filter_operator : partition_filter_operators) {
      partition_filters.emplace_back(partition_filter_operator->evaluate());
   }
   return action->executeAndOrder(table, std::move(partition_filters));
}
}  // namespace

LegacyResultProducer::LegacyResultProducer(
   arrow::acero::ExecPlan* plan,
   const std::vector<silo::schema::ColumnIdentifier>& columns,
   std::shared_ptr<const storage::Table> table,
   std::shared_ptr<std::vector<std::unique_ptr<filter::operators::Operator>>>
      partition_filter_operators,
   const actions::Action* action,
   size_t materialization_cutoff
)
    : arrow::acero::ExecNode(plan, {}, {}, columnsToArrowSchema(columns)),
      materialization_cutoff(materialization_cutoff) {
   query_result = createLegacyQueryResult(*partition_filter_operators, action, table);
   for (auto& field : output_schema_->fields()) {
      field_names.emplace_back(&field->name());
   }
   prepareOutputArrays();
}

void LegacyResultProducer::prepareOutputArrays() {
   for (auto& field : output_schema_->fields()) {
      array_builders.emplace_back(field->type());
   }
}

arrow::Status LegacyResultProducer::flushOutput() {
   std::vector<arrow::Datum> data;
   data.reserve(array_builders.size());
   for (auto& array : array_builders) {
      arrow::Datum datum;
      ARROW_ASSIGN_OR_RAISE(datum, array.toDatum());
      data.push_back(std::move(datum));
   }

   arrow::ExecBatch exec_batch;
   ARROW_ASSIGN_OR_RAISE(exec_batch, arrow::compute::ExecBatch::Make(data));
   exec_batch.index = num_batches_produced++;
   ARROW_RETURN_NOT_OK(this->output_->InputReceived(this, exec_batch));
   return arrow::Status::OK();
}

arrow::Status LegacyResultProducer::produce() {
   SPDLOG_TRACE("LegacyResultProducer::produce");
   size_t num_rows = 0;
   std::optional<QueryResultEntry> row;
   while ((row = query_result.next())) {
      ++num_rows;
      for (size_t field_idx = 0; field_idx < field_names.size(); ++field_idx) {
         const auto field_name = field_names.at(field_idx);
         const common::JsonValueType& field_value = row.value().fields.at(*field_name);

         auto status = array_builders.at(field_idx).insert(field_value);
         if (status.IsCapacityError()) {
            throw std::runtime_error(fmt::format(
               "Response size too large. Materializing {} rows required more than allowed {} bytes",
               materialization_cutoff,
               INT32_MAX
            ));
         }
         ARROW_RETURN_NOT_OK(status);
      }

      if (num_rows > materialization_cutoff) {
         ARROW_RETURN_NOT_OK(flushOutput());
         num_rows = 0;
      }
   }
   if (num_rows > 0) {
      ARROW_RETURN_NOT_OK(flushOutput());
   }
   SPDLOG_TRACE("LegacyResultProducer::end");
   return arrow::Status::OK();
}

}  // namespace silo::query_engine::exec_node
