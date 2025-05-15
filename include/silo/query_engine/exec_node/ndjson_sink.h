#pragma once

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <fmt/format.h>

namespace silo::query_engine::exec_node {

class NdjsonSink : public arrow::acero::ExecNode {
   std::ostream* output_stream;
   int batches_written = 0;
   std::optional<int> total_batches_from_input = std::nullopt;

  public:
   NdjsonSink(arrow::acero::ExecPlan* plan, std::ostream* stream, arrow::acero::ExecNode* input)
       : arrow::acero::ExecNode(plan, {input}, {"input"}, nullptr),
         output_stream(stream) {}

   virtual const char* kind_name() const override { return "NdjsonSinkNode"; }

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
         *output_stream << scalar.value;
         return arrow::Status::OK();
      }

      arrow::Status Visit(const arrow::StringScalar& scalar) override {
         // TODO maybe instead:
         // nlohmann::json j = scalar.ToString();
         // *output_stream << j;
         *output_stream << "\"" << scalar.ToString() << "\"";
         return arrow::Status::OK();
      }

      arrow::Status Visit(const arrow::BooleanScalar& scalar) override {
         *output_stream << (scalar.value ? "true" : "false");
         return arrow::Status::OK();
      }
   };

   arrow::Status writeRecordBatchAsNdjson(std::shared_ptr<arrow::RecordBatch> record_batch) {
      size_t row_count = record_batch->num_rows();
      size_t column_count = record_batch->num_columns();

      std::vector<std::string> prepared_column_strings_for_json_attributes;
      for (const auto& column : record_batch->schema()->field_names()) {
         prepared_column_strings_for_json_attributes.emplace_back(fmt::format("\"{}\":", column));
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

   arrow::Status InputReceived(arrow::acero::ExecNode* input, arrow::compute::ExecBatch batch)
      override {
      std::shared_ptr<arrow::RecordBatch> record_batch;
      ARROW_ASSIGN_OR_RAISE(record_batch, batch.ToRecordBatch(input->output_schema()));
      ARROW_RETURN_NOT_OK(writeRecordBatchAsNdjson(record_batch));
      batches_written += 1;
      if (batches_written == total_batches_from_input) {
         output_stream = nullptr;
      }
      return arrow::Status::OK();
   }

   arrow::Status InputFinished(arrow::acero::ExecNode* input, int total_batches) override {
      this->total_batches_from_input = total_batches;
      if (total_batches == 0) {
         output_stream = nullptr;
      }
      return arrow::Status::OK();
   }

   arrow::Status StartProducing() override { return arrow::Status::OK(); }
   arrow::Status StopProducing() override { return arrow::Status::OK(); }
   arrow::Status StopProducingImpl() override { return arrow::Status::OK(); }

   void ResumeProducing(arrow::acero::ExecNode* output, int32_t counter) override {}

   void PauseProducing(arrow::acero::ExecNode* output, int32_t counter) override {}
};

}  // namespace silo::query_engine::exec_node