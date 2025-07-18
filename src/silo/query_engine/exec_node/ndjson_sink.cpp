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

template <size_t PARALLEL_STREAMS>
struct ParallelStringStream {
   std::array<std::stringstream, PARALLEL_STREAMS> streams;

   void operator<<(std::string_view bytes) {
      for (size_t i = 0; i < PARALLEL_STREAMS; ++i) {
         streams[i] << bytes;
      }
   }

   void operator>>(std::ostream& output) {
      for (size_t i = 0; i < PARALLEL_STREAMS; ++i) {
         output << streams[i].rdbuf();
      }
   }
};

template <size_t PARALLEL_LINES>
class ArrayToJsonTypeVisitor : public arrow::ArrayVisitor {
   ParallelStringStream<PARALLEL_LINES>& output_stream;
   size_t row_base;

  public:
   ArrayToJsonTypeVisitor(ParallelStringStream<PARALLEL_LINES>& output_stream)
       : output_stream(output_stream),
         row_base(0) {}

   void next() { row_base += PARALLEL_LINES; }

   arrow::Status Visit(const arrow::Int32Array& array) override {
      EVOBENCH_SCOPE("ArrayToJsonTypeVisitor", "Int32Array");
      for (size_t i = 0; i < PARALLEL_LINES; ++i) {
         if (array.IsNull(row_base + i)) {
            output_stream.streams.at(i) << "null";
         } else {
            output_stream.streams.at(i) << array.GetView(row_base + i);
         }
      }
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::Int64Array& array) override {
      EVOBENCH_SCOPE("ArrayToJsonTypeVisitor", "Int64Array");
      for (size_t i = 0; i < PARALLEL_LINES; ++i) {
         if (array.IsNull(row_base + i)) {
            output_stream.streams.at(i) << "null";
         } else {
            output_stream.streams.at(i) << array.GetView(row_base + i);
         }
      }
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::DoubleArray& array) override {
      EVOBENCH_SCOPE("ArrayToJsonTypeVisitor", "DoubleArray");
      for (size_t i = 0; i < PARALLEL_LINES; ++i) {
         if (array.IsNull(row_base + i)) {
            output_stream.streams.at(i) << "null";
         } else {
            nlohmann::json j = array.GetView(row_base + i);
            output_stream.streams.at(i) << j;
         }
      }
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::FloatArray& array) override {
      EVOBENCH_SCOPE("ArrayToJsonTypeVisitor", "FloatArray");
      for (size_t i = 0; i < PARALLEL_LINES; ++i) {
         if (array.IsNull(row_base + i)) {
            output_stream.streams.at(i) << "null";
         } else {
            nlohmann::json j = array.GetView(row_base + i);
            output_stream.streams.at(i) << j;
         }
      }
      return arrow::Status::OK();
   }

   static inline void serializeStringToStream(
      std::string_view string,
      std::ostream& output_stream
   ) {
      output_stream << "\"";
      for (char c : string) {
         switch (c) {
            case '"':
               output_stream << "\\\"";
               break;
            case '\\':
               output_stream << "\\\\";
               break;
            case '\b':
               output_stream << "\\b";
               break;
            case '\f':
               output_stream << "\\f";
               break;
            case '\n':
               output_stream << "\\n";
               break;
            case '\r':
               output_stream << "\\r";
               break;
            case '\t':
               output_stream << "\\t";
               break;
            default:
               if (c < 0x20) {
                  output_stream << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                                << static_cast<int>(c);
               } else {
                  output_stream << c;
               }
         }
      }
      output_stream << "\"";
   }

   arrow::Status Visit(const arrow::StringArray& array) override {
      EVOBENCH_SCOPE("ArrayToJsonTypeVisitor", "StringArray");
      for (size_t i = 0; i < PARALLEL_LINES; ++i) {
         if (array.IsNull(row_base + i)) {
            output_stream.streams.at(i) << "null";
         } else {
            serializeStringToStream(array.GetView(row_base + i), output_stream.streams.at(i));
         }
      }
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::BooleanArray& array) override {
      EVOBENCH_SCOPE("ArrayToJsonTypeVisitor", "BooleanArray");
      for (size_t i = 0; i < PARALLEL_LINES; ++i) {
         if (array.IsNull(row_base + i)) {
            output_stream.streams.at(i) << "null";
         } else {
            output_stream.streams.at(i) << (array.GetView(row_base + i) ? "true" : "false");
         }
      }
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
   size_t row_count = batch.length;
   size_t column_count = schema->fields().size();
   std::vector<std::shared_ptr<arrow::Array>> arrays;
   for (const auto& datum : batch.values) {
      SILO_ASSERT(datum.is_array());
      arrays.emplace_back(datum.make_array());
   }
   std::vector<std::string> prepared_column_strings_for_json_attributes;
   bool first_column = true;
   for (const auto& column_name : schema->fields()) {
      nlohmann::json column_name_json = column_name->name();
      std::string json_formatted_column_name;
      if (first_column)
         json_formatted_column_name += ",";
      first_column = false;
      json_formatted_column_name = column_name_json.dump();
      json_formatted_column_name += ":";
      prepared_column_strings_for_json_attributes.emplace_back(json_formatted_column_name);
   }
   constexpr size_t parallel_lines = 8;
   ParallelStringStream<parallel_lines> ndjson_line_streams;
   ArrayToJsonTypeVisitor<parallel_lines> my_visitor(ndjson_line_streams);
   for (size_t row_idx_base = 0; row_idx_base + parallel_lines < row_count;
        row_idx_base += parallel_lines) {
      ndjson_line_streams << "{";
      for (size_t column_idx = 0; column_idx < column_count; column_idx++) {
         const auto& column = arrays.at(column_idx);
         ndjson_line_streams << prepared_column_strings_for_json_attributes.at(column_idx);

         EVOBENCH_SCOPE("QueryPlan", "writeBatchAsNdjson(innermost_scope)");
         ARROW_RETURN_NOT_OK(column->Accept(&my_visitor));
      }
      ndjson_line_streams << "}\n";
      {
         EVOBENCH_SCOPE("QueryPlan", "sendDataToOutputStream");
         ndjson_line_streams >> *output_stream;
      }
      my_visitor.next();
   }
   if (!*output_stream) {
      return arrow::Status::IOError("Could not write to network stream");
   }
   SPDLOG_TRACE("writeBatchAsNdjson_end");

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
