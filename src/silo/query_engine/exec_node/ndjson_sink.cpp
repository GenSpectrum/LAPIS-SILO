#include "silo/query_engine/exec_node/ndjson_sink.h"
#include "evobench/evobench.hpp"

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

   arrow::Status Visit(const arrow::Int64Scalar& scalar) override {
      *output_stream << scalar.value;
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::DoubleScalar& scalar) override {
      nlohmann::json j = scalar.value;
      *output_stream << j;
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::FloatScalar& scalar) override {
      nlohmann::json j = scalar.value;
      *output_stream << j;
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::StringScalar& scalar) override {
      for (char c : scalar.view()) {
         switch (c) {
            case '"':  *output_stream << "\\\""; break;
            case '\\': *output_stream << "\\\\"; break;
            case '\b': *output_stream << "\\b"; break;
            case '\f': *output_stream << "\\f"; break;
            case '\n': *output_stream << "\\n"; break;
            case '\r': *output_stream << "\\r"; break;
            case '\t': *output_stream << "\\t"; break;
            default:
               if (c < 0x20) {
                  *output_stream << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
               } else {
                  *output_stream << c;
               }
         }
      }
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::BooleanScalar& scalar) override {
      *output_stream << (scalar.value ? "true" : "false");
      return arrow::Status::OK();
   }
};
}  // namespace

arrow::Status writeBatchAsNdjson(
   arrow::compute::ExecBatch batch,
   const std::shared_ptr<arrow::Schema>& schema,
   std::ostream* output_stream
) {
   EVOBENCH_SCOPE("QueryPlan", "writeBatchAsNdjson");
   ARROW_ASSIGN_OR_RAISE(auto record_batch, batch.ToRecordBatch(schema));
   size_t row_count = record_batch->num_rows();
   size_t column_count = record_batch->num_columns();
   std::vector<std::string> prepared_column_strings_for_json_attributes;
   auto column_names = record_batch->schema()->fields();
   for (const auto& column_name : column_names) {
      nlohmann::json json_formatted_column_name = column_name->name();
      prepared_column_strings_for_json_attributes.emplace_back(json_formatted_column_name.dump());
   }
   for (size_t row_idx = 0; row_idx < row_count; row_idx++) {
      std::stringstream ndjson_line_stream;
      ndjson_line_stream << "{";
      for (size_t column_idx = 0; column_idx < column_count; column_idx++) {
         if (column_idx != 0) {
            ndjson_line_stream << ",";
         }
         const auto& column = record_batch->columns().at(column_idx);
         ndjson_line_stream << prepared_column_strings_for_json_attributes.at(column_idx);
         ndjson_line_stream << ":";

         if (column->IsNull(row_idx)) {
            ndjson_line_stream << "null";
         } else {
            ARROW_ASSIGN_OR_RAISE(const auto& scalar, column->GetScalar(row_idx));
            ScalarToJsonTypeVisitor my_visitor(&ndjson_line_stream);
            ARROW_RETURN_NOT_OK(scalar->Accept(&my_visitor));
         }
      }
      ndjson_line_stream << "}\n";
      {
         EVOBENCH_SCOPE("QueryPlan", "sendDataToOutputStream");
         *output_stream << ndjson_line_stream.rdbuf();
      }
      if (!*output_stream) {
         return arrow::Status::IOError("Could not write to network stream");
      }
   }

   return arrow::Status::OK();
}

arrow::Status createGenerator(
   arrow::acero::ExecPlan* plan,
   arrow::acero::ExecNode* input,
   arrow::AsyncGenerator<std::optional<arrow::ExecBatch>>* generator
) {
   arrow::acero::SinkNodeOptions options{
      generator, arrow::acero::BackpressureOptions::DefaultBackpressure()
   };
   ARROW_RETURN_NOT_OK(
      arrow::acero::MakeExecNode(std::string{"sink"}, plan, {input}, options).status()
   );
   return arrow::Status::OK();
}

}  // namespace silo::query_engine::exec_node
