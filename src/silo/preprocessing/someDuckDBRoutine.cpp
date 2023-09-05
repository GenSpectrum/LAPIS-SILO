#include "silo/preprocessing/someDuckDBRoutine.h"

#include <duckdb.hpp>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

void silo::executeDuckDBRoutine(std::string_view file_name) {
   duckdb::DuckDB duckDb;
   duckdb::Connection duckdb_connection(duckDb);
   std::string query = ::fmt::format(
      "SELECT * FROM read_csv_auto('somefile.tsv', HEADER=TRUE, all_varchar=TRUE);", file_name
   );
   SPDLOG_INFO("executing duckdb query: {}", query);
   auto res = duckdb_connection.Query(query);
   SPDLOG_INFO("duckdb Result: {}", res->ToString());
   query = "COPY metadata_table TO 'FILEoutputFILE.csv' (HEADER, DELIMITER ',');";
   SPDLOG_INFO("executing duckdb query: {}", query);
   res = duckdb_connection.Query(query);
   SPDLOG_INFO("duckdb Result: {}", res->ToString());
}