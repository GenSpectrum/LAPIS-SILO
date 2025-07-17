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

template <size_t BATCH_SIZE>
struct BatchedStringStream {
   std::array<std::stringstream, BATCH_SIZE> streams;

   void operator<<(std::string_view bytes) {
      for (size_t i = 0; i < BATCH_SIZE; ++i) {
         streams[i] << bytes;
      }
   }

   void operator>>(std::ostream& output) {
      for (size_t i = 0; i < BATCH_SIZE; ++i) {
         output << streams[i].rdbuf();
      }
   }
};

template <size_t BATCH_SIZE>
class ArrayToJsonTypeVisitor : public arrow::ArrayVisitor {
   BatchedStringStream<BATCH_SIZE>& output_stream;
   size_t& row_base;

  public:
   ArrayToJsonTypeVisitor(BatchedStringStream<BATCH_SIZE>& output_stream, size_t& row_base)
       : output_stream(output_stream),
         row_base(row_base) {}

   arrow::Status Visit(const arrow::Int32Array& array) override {
      for (size_t i = 0; i < BATCH_SIZE; ++i) {
         if (array.IsNull(row_base + i)) {
            output_stream.streams.at(i) << "null";
         } else {
            output_stream.streams.at(i) << array.GetView(row_base + i);
         }
      }
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::Int64Array& array) override {
      for (size_t i = 0; i < BATCH_SIZE; ++i) {
         if (array.IsNull(row_base + i)) {
            output_stream.streams.at(i) << "null";
         } else {
            output_stream.streams.at(i) << array.GetView(row_base + i);
         }
      }
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::DoubleArray& array) override {
      for (size_t i = 0; i < BATCH_SIZE; ++i) {
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
      for (size_t i = 0; i < BATCH_SIZE; ++i) {
         if (array.IsNull(row_base + i)) {
            output_stream.streams.at(i) << "null";
         } else {
            nlohmann::json j = array.GetView(row_base + i);
            output_stream.streams.at(i) << j;
         }
      }
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::StringArray& array) override {
      for (size_t i = 0; i < BATCH_SIZE; ++i) {
         if (array.IsNull(row_base + i)) {
            output_stream.streams.at(i) << "null";
         } else {
            std::string_view value = array.GetView(row_base + i);
            nlohmann::json json_string = value;
            output_stream.streams.at(i) << json_string.dump();
         }
      }
      return arrow::Status::OK();
   }

   arrow::Status Visit(const arrow::BooleanArray& array) override {
      for (size_t i = 0; i < BATCH_SIZE; ++i) {
         if (array.IsNull(row_base + i)) {
            output_stream.streams.at(i) << "null";
         } else {
            output_stream.streams.at(i) << (array.GetView(row_base + i) ? "true" : "false");
         }
      }
      return arrow::Status::OK();
   }
};

template <size_t BATCH_SIZE>
void sendJsonLinesInBatches(
   size_t& row_idx_base,
   size_t row_count,
   const std::vector<std::string>& prepared_column_strings_for_json_attributes,
   const std::vector<std::shared_ptr<arrow::Array>>& column_arrays,
   std::ostream& output_stream
) {
   BatchedStringStream<BATCH_SIZE> ndjson_line_streams;
   ArrayToJsonTypeVisitor<BATCH_SIZE> my_visitor(ndjson_line_streams, row_idx_base);
   size_t column_count = column_arrays.size();
   for (; row_idx_base + BATCH_SIZE <= row_count; row_idx_base += BATCH_SIZE) {
      ndjson_line_streams << "{";
      for (size_t column_idx = 0; column_idx < column_count; column_idx++) {
         const auto& column_array = column_arrays.at(column_idx);
         ndjson_line_streams << prepared_column_strings_for_json_attributes.at(column_idx);

         (void)column_array->Accept(&my_visitor);
      }
      ndjson_line_streams << "}\n";
      {
         EVOBENCH_SCOPE("QueryPlan", "sendDataToOutputStream");
         ndjson_line_streams >> output_stream;
      }
   }
   if constexpr (BATCH_SIZE > 1) {
      // Send remaining lines
      sendJsonLinesInBatches<1>(
         row_idx_base,
         row_count,
         prepared_column_strings_for_json_attributes,
         column_arrays,
         output_stream
      );
   }
}

}  // namespace

arrow::Status writeBatchAsNdjson(
   arrow::compute::ExecBatch batch,
   const std::shared_ptr<arrow::Schema>& schema,
   std::ostream* output_stream
) {
   EVOBENCH_SCOPE("QueryPlan", "writeBatchAsNdjson");
   size_t row_count = batch.length;
   std::vector<std::shared_ptr<arrow::Array>> column_arrays;
   for (const auto& datum : batch.values) {
      SILO_ASSERT(datum.is_array());
      column_arrays.emplace_back(datum.make_array());
   }
   std::vector<std::string> prepared_column_strings_for_json_attributes;
   bool first_column = true;
   for (const auto& column_name : schema->fields()) {
      nlohmann::json column_name_json = column_name->name();
      std::string json_formatted_column_name;
      if (!first_column)
         json_formatted_column_name += ",";
      first_column = false;
      json_formatted_column_name += column_name_json.dump();
      json_formatted_column_name += ":";
      prepared_column_strings_for_json_attributes.emplace_back(json_formatted_column_name);
   }
   size_t row_idx = 0;
   constexpr size_t BATCH_SIZE = 8;
   sendJsonLinesInBatches<BATCH_SIZE>(
      row_idx, row_count, prepared_column_strings_for_json_attributes, column_arrays, *output_stream
   );
   if (!*output_stream) {
      return arrow::Status::IOError("Could not write to network stream");
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
