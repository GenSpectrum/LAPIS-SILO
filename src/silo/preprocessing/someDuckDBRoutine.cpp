#include "silo/preprocessing/someDuckDBRoutine.h"

#include <duckdb.hpp>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

void silo::executeDuckDBRoutine(std::string_view file_name) {
   duckdb::DuckDB duckDb;
   duckdb::Connection duckdb_connection(duckDb);
   const ::std::string query = ::fmt::format(
      "SELECT * FROM read_csv_auto('somefile.tsv', HEADER=TRUE, "
      "DELIM='\\t', dateformat='%x', sample_size=1, column_types={{'second_header':"
      "'int64', 'date': 'date', 'some_header': 'VARCHAR', 'date2': 'int64'}});",
      file_name
   );
   SPDLOG_INFO("executing duckdb query: {}", query);
   auto res = duckdb_connection.Query(query);
   SPDLOG_INFO("duckdb Result: {}", res->ToString());
   duckdb_connection.Query("COPY metadata_table TO 'FILEoutputFILE.csv' (HEADER, DELIMITER ',');");
}