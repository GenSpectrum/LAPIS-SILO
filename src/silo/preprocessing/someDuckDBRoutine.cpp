#include "silo/preprocessing/someDuckDBRoutine.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <duckdb.hpp>

#include "silo/common/zstd_compressor.h"

silo::ZstdCompressor compressors("ABC");
duckdb::string_t buffer(compressor.getSizeBound());
duckdb::string_t zstdCompressOneGenome(
   const duckdb::string_t uncompressed,
   duckdb::string_t genome_name
) {
   compressor.compress(
      uncompressed.GetData(), uncompressed.GetSize(), buffer.GetDataWriteable(), buffer.GetSize()
   );
   return buffer;
}

void executeQuery(duckdb::Connection& db, std::string sql_query) {
   SPDLOG_INFO("executing duckdb query: {}", sql_query);
   auto res = db.Query(sql_query);
   SPDLOG_INFO("duckdb Result: {}", res->ToString());
}

void silo::executeDuckDBRoutine(std::string_view file_name) {
   duckdb::DuckDB duckDb;
   duckdb::Connection duckdb_connection(duckDb);

   duckdb_connection.CreateScalarFunction<duckdb::string_t, duckdb::string_t, duckdb::string_t>(
      "compressGene", &zstdCompressOneGenome
   );

   executeQuery(duckdb_connection, "INSTALL json;");
   executeQuery(duckdb_connection, "LOAD json;");

   executeQuery(
      duckdb_connection,
      ::fmt::format(
         "CREATE TABLE ndjson AS SELECT * FROM read_json_auto('{}', format=unstructured);",
         file_name
      )
   );

   executeQuery(duckdb_connection, ::fmt::format("SELECT * FROM ndjson;", file_name));
}