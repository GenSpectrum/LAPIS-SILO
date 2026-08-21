#include "rhydb/query_engine/command/insert_command.h"

#include <cstddef>
#include <sstream>
#include <utility>

#include <nlohmann/json.hpp>

#include "rhydb/database.h"
#include "rhydb/query_engine/exec_node/ndjson_sink.h"
#include "rhydb/query_engine/planner.h"
#include "rhydb/query_engine/query_plan.h"
#include "rhydb/storage/table.h"

namespace rhydb::query_engine::command {

InsertCommand::InsertCommand(operators::QueryNodePtr source_query, schema::TableName target_table)
    : source_query_(std::move(source_query)),
      target_table_(std::move(target_table)) {}

nlohmann::json InsertCommand::execute(Database& database) {
   // Plan and run the source query, streaming its result rows out as NDJSON. Reusing the NDJSON
   // sink and the existing NDJSON append path keeps a single JSON representation shared by both
   // sides, so every column type that a query can emit and the append path can ingest round-trips
   // without a bespoke Arrow-to-column bridge.
   auto query_plan = Planner::planQuery(
      std::move(source_query_), database.tables, config::QueryOptions{}, "insertInto"
   );

   std::stringstream ndjson_buffer;
   exec_node::NdjsonSink output_sink{&ndjson_buffer, query_plan.results_schema};
   constexpr uint64_t DEFAULT_TIMEOUT_SECONDS = 120;
   query_plan.executeAndWrite(output_sink, DEFAULT_TIMEOUT_SECONDS);

   auto& target_table = *database.tables.at(target_table_);
   const size_t rows_before = target_table.row_layout.numRows();
   database.appendData(target_table_, ndjson_buffer);
   const size_t rows_after = target_table.row_layout.numRows();

   return {{"insertedRows", rows_after - rows_before}};
}

}  // namespace rhydb::query_engine::command
