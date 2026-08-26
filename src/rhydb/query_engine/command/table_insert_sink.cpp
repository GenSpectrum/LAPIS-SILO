#include "rhydb/query_engine/command/table_insert_sink.h"

#include <string>
#include <utility>

#include "rhydb/append/ndjson_line_reader.h"

namespace rhydb::query_engine::command {

TableInsertSink::TableInsertSink(
   std::shared_ptr<storage::Table> target_table,
   std::shared_ptr<arrow::Schema> results_schema
)
    : ndjson_sink(&batch_buffer, std::move(results_schema)),
      table_inserter(std::move(target_table)),
      insert_stream(table_inserter) {}

arrow::Status TableInsertSink::writeBatch(const arrow::compute::ExecBatch& batch) {
   batch_buffer.str(std::string{});
   batch_buffer.clear();

   ARROW_RETURN_NOT_OK(ndjson_sink.writeBatch(batch));

   append::NdjsonLineReader batch_lines{batch_buffer};
   insert_stream.insertAll(batch_lines);
   return arrow::Status::OK();
}

arrow::Status TableInsertSink::finish() {
   ARROW_RETURN_NOT_OK(ndjson_sink.finish());
   [[maybe_unused]] const auto commit = table_inserter.commit();
   return arrow::Status::OK();
}

}  // namespace rhydb::query_engine::command
