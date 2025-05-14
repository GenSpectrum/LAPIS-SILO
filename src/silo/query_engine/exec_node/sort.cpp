#include "silo/query_engine/exec_node/sort.h"

#include "silo/query_engine/exec_node/arrow_util.h"
//
//namespace silo::query_engine::exec_node {
//
//Sort::Sort(
//   arrow::acero::ExecPlan* plan,
//   const std::vector<silo::schema::ColumnIdentifier>& columns,  // TODO const & ?
//   std::shared_ptr<Database> database,
//   std::shared_ptr<Query> query
//)
//    : arrow::acero::ExecNode(plan, {}, {}, columnsToArrowSchema(columns)) {
//   query_result = createLegacyQueryResult(*query, *database);
//   for (auto& field : output_schema_.get()->fields()) {
//      field_names.emplace_back(&field->name());
//   }
//   prepareOutputArrays();
//}
//
//void LegacyResultProducer::prepareOutputArrays() {
//   for (auto& field : output_schema_.get()->fields()) {
//      arrays.emplace_back(field->type());
//   }
//}
//
//arrow::Status LegacyResultProducer::flushOutput() {
//   std::vector<arrow::Datum> data;
//   for (auto& array : arrays) {
//      data.push_back((std::move(array)).toDatum());
//   }
//   arrow::ExecBatch exec_batch;
//   ARROW_ASSIGN_OR_RAISE(exec_batch, arrow::compute::ExecBatch::Make(data));
//   ARROW_RETURN_NOT_OK(this->output_->InputReceived(this, exec_batch));
//   return arrow::Status::OK();
//}
//
//// TODO make configurable
//static constexpr size_t MATERIALIZATION_CUTOFF = 50000;
//
//arrow::Status LegacyResultProducer::produce() {
//   size_t num_rows = 0;
//   std::optional<QueryResultEntry> row;
//   while ((row = query_result.next())) {
//      ++num_rows;
//      for (size_t field_idx = 0; field_idx < field_names.size(); ++field_idx) {
//         const auto field_name = field_names.at(field_idx);
//         const common::JsonValueType& field_value = row.value().fields.at(*field_name);
//
//         auto status = arrays.at(field_idx).insert(field_value);
//         if (status.IsCapacityError()) {
//            throw std::runtime_error(fmt::format(
//               "Response size too large. Materializing {} rows required more than allowed {} bytes",
//               MATERIALIZATION_CUTOFF,
//               INT32_MAX
//            ));
//         }
//         ARROW_RETURN_NOT_OK(status);
//      }
//
//      if (num_rows > MATERIALIZATION_CUTOFF) {
//         ARROW_RETURN_NOT_OK(flushOutput());
//      }
//   }
//   ARROW_RETURN_NOT_OK(flushOutput());
//   return arrow::Status::OK();
//}
//
//arrow::Status LegacyResultProducer::StartProducing() {
//   return produce();
//}
//
//arrow::Status LegacyResultProducer::StopProducing() {
//   return arrow::Status::OK();
//}
//
//}  // namespace silo::query_engine::exec_node
