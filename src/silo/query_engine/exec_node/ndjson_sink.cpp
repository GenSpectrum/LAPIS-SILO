#include "silo/query_engine/exec_node/ndjson_sink.h"
#include "evobench/evobench.hpp"

#include <arrow/acero/options.h>
#include <arrow/array.h>
#include <arrow/array/array_binary.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

namespace silo::query_engine::exec_node {

namespace {
class ArrayToJsonTypeVisitor : public arrow::ArrayVisitor {
   std::ostream* output_stream;
   size_t row;

  public:
   ArrayToJsonTypeVisitor(std::ostream* output_stream, size_t row)
       : output_stream(output_stream), row(row) {}

   arrow::Status Visit(const arrow::Int32Array& array) override {
      EVOBENCH_SCOPE("ArrayToJsonTypeVisitor", "Int32Array");
      *output_stream << array.GetView(row);
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::Int64Array& array) override {
      EVOBENCH_SCOPE("ArrayToJsonTypeVisitor", "Int64Array");
      *output_stream << array.GetView(row);
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::DoubleArray& array) override {
      EVOBENCH_SCOPE("ArrayToJsonTypeVisitor", "DoubleArray");
      nlohmann::json j = array.GetView(row);
      *output_stream << j;
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::FloatArray& array) override {
      EVOBENCH_SCOPE("ArrayToJsonTypeVisitor", "FloatArray");
      nlohmann::json j = array.GetView(row);
      *output_stream << j;
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::StringArray& array) override {
      EVOBENCH_SCOPE("ArrayToJsonTypeVisitor", "StringArray");
      for (char c : array.GetView(row)) {
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

   arrow::Status Visit(const arrow::BooleanArray& array) override {
      EVOBENCH_SCOPE("ArrayToJsonTypeVisitor", "BooleanArray");
      *output_stream << (array.GetView(row) ? "true" : "false");
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
            EVOBENCH_SCOPE("QueryPlan", "writeBatchAsNdjson(innermost_scope)");
            ArrayToJsonTypeVisitor my_visitor(&ndjson_line_stream, row_idx);
            ARROW_RETURN_NOT_OK(column->Accept(&my_visitor));
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
