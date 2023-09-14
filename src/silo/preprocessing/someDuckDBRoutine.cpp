#include "silo/preprocessing/someDuckDBRoutine.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <duckdb.hpp>

#include "silo/common/zstd_compressor.h"
#include "silo/storage/reference_genomes.h"

void executeQuery(duckdb::Connection& db, std::string sql_query) {
   SPDLOG_INFO("executing duckdb query: {}", sql_query);
   auto res = db.Query(sql_query);
   SPDLOG_INFO("duckdb Result: {}", res->ToString());
}

void silo::executeDuckDBRoutine(
   const silo::Database& database,
   const silo::ReferenceGenomes& reference_genomes,
   std::string_view file_name
) {
   std::unordered_map<std::string_view, silo::ZstdCompressor> nuc_compressors;
   std::unordered_map<std::string_view, std::string> nuc_buffers;
   for (const auto& [name, sequence] : reference_genomes.raw_nucleotide_sequences) {
      nuc_compressors.emplace(name, silo::ZstdCompressor(sequence));
      nuc_buffers.emplace(name, std::string(nuc_compressors.at(name).getSizeBound(), '\0'));
   }

   std::function<duckdb::string_t(duckdb::string_t, duckdb::string_t)> zstdCompressOneGenome =
      [&](const duckdb::string_t uncompressed, duckdb::string_t genome_name) {
         std::string& buffer = nuc_buffers.at(genome_name.GetString());
         size_t size_or_error_code =
            nuc_compressors.at(genome_name.GetString())
               .compress(
                  uncompressed.GetData(), uncompressed.GetSize(), buffer.data(), buffer.size()
               );
         // TODO check size_or_error_code
         duckdb_value varchar = duckdb_create_varchar_length(buffer.data(), size_or_error_code);
         return duckdb::string_t{duckdb_get_varchar(varchar)};
      };

   duckdb::DuckDB duckDb;
   duckdb::Connection duckdb_connection(duckDb);

   duckdb_connection.CreateScalarFunction<duckdb::string_t, duckdb::string_t, duckdb::string_t>(
      "compressGene",
      zstdCompressOneGenome.target<duckdb::string_t(duckdb::string_t, duckdb::string_t)>()
   );

   executeQuery(
      duckdb_connection,
      ::fmt::format(
         "CREATE TABLE ndjson AS SELECT * FROM 'sample.ndjson.zst' "
         "LIMIT 1;",
         file_name
      )
   );
   executeQuery(
      duckdb_connection, "SELECT compressGene(alignedNucleotideSequences.main, 'main') FROM ndjson;"
   );
}
