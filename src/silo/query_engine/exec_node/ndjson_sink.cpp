#include "silo/query_engine/exec_node/ndjson_sink.h"

#include <arrow/acero/options.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

namespace silo::query_engine::exec_node {

namespace {
class ScalarToJsonTypeVisitor : public arrow::ScalarVisitor {
   std::ostream* output_stream;

  public:
   ScalarToJsonTypeVisitor(std::ostream* output_stream)
       : output_stream(output_stream) {}

   arrow::Status Visit(const arrow::Int32Scalar& scalar) override {
      *output_stream << scalar.value;
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::DoubleScalar& scalar) override {
      nlohmann::json j = scalar.value;
      *output_stream << j;
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::StringScalar& scalar) override {
      nlohmann::json j = scalar.ToString();
      *output_stream << j;
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::BooleanScalar& scalar) override {
      *output_stream << (scalar.value ? "true" : "false");
      return arrow::Status::OK();
   }
};
}  // namespace

arrow::Status NdjsonSink::writeRecordBatchAsNdjson(std::shared_ptr<arrow::RecordBatch> record_batch
) {
   size_t row_count = record_batch->num_rows();
   size_t column_count = record_batch->num_columns();
   std::vector<std::string> prepared_column_strings_for_json_attributes;
   auto column_names = record_batch->schema()->fields();
   for (const auto& column_name : column_names) {
      const auto json_formatted_column_name = fmt::format("\"{}\":", column_name->name());
      prepared_column_strings_for_json_attributes.emplace_back(json_formatted_column_name);
   }
   for (size_t row_idx = 0; row_idx < row_count; row_idx++) {
      *output_stream << "{";
      for (size_t column_idx = 0; column_idx < column_count; column_idx++) {
         if (column_idx != 0) {
            *output_stream << ",";
         }
         const auto& column = record_batch->columns().at(column_idx);
         *output_stream << prepared_column_strings_for_json_attributes.at(column_idx);

         if (column->IsNull(row_idx)) {
            *output_stream << "null";
         } else {
            ARROW_ASSIGN_OR_RAISE(const auto& scalar, column->GetScalar(row_idx));
            ScalarToJsonTypeVisitor my_visitor(output_stream);
            ARROW_RETURN_NOT_OK(scalar->Accept(&my_visitor));
         }
      }
      *output_stream << "}\n";
   }

   return arrow::Status::OK();
}

arrow::Status NdjsonSink::InputReceived(
   arrow::acero::ExecNode* input,
   arrow::compute::ExecBatch batch
) {
   SPDLOG_TRACE("NdjsonSink::InputReceived");
   try {
      std::shared_ptr<arrow::RecordBatch> record_batch;
      ARROW_ASSIGN_OR_RAISE(record_batch, batch.ToRecordBatch(input->output_schema()));
      ARROW_RETURN_NOT_OK(writeRecordBatchAsNdjson(record_batch));
      batches_written += 1;
      if (batches_written == total_batches_from_input) {
         output_stream->flush();
         output_stream = nullptr;
      }
      return arrow::Status::OK();
   } catch (const std::runtime_error& error) {
      const auto error_message = fmt::format(
         "NdjsonSink::InputReceived, exception thrown when not expected: {}", error.what()
      );
      SPDLOG_ERROR(error_message);
      return arrow::Status::ExecutionError(error_message);
   }
}

arrow::Status NdjsonSink::StartProducing() {
   SPDLOG_TRACE("NdjsonSink::StartProducing");
   return arrow::Status::OK();
}

arrow::Status NdjsonSink::StopProducing() {
   SPDLOG_TRACE("NdjsonSink::StopProducing");
   return arrow::Status::OK();
}

arrow::Status NdjsonSink::InputFinished(arrow::acero::ExecNode* input, int total_batches) {
   SPDLOG_TRACE("NdjsonSink::InputFinished({})", total_batches);
   total_batches_from_input = total_batches;
   if (batches_written == total_batches_from_input) {
      output_stream->flush();
      output_stream = nullptr;
   }
   return arrow::Status::OK();
}

}  // namespace silo::query_engine::exec_node
