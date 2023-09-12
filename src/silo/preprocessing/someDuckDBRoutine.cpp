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
   std::unordered_map<std::string_view, duckdb::string_t> nuc_buffers;
   for (const auto& [name, sequence] : reference_genomes.raw_nucleotide_sequences) {
      nuc_compressors.emplace(name, silo::ZstdCompressor(sequence));
      nuc_buffers.emplace(name, duckdb::string_t(nuc_compressors.at(name).getSizeBound()));
   }

   std::function<duckdb::string_t(duckdb::string_t, duckdb::string_t)> zstdCompressOneGenome =
      [&](const duckdb::string_t uncompressed, duckdb::string_t genome_name) {
         duckdb::string_t& buffer = nuc_buffers.at(genome_name.GetString());
         nuc_compressors.at(genome_name.GetString())
            .compress(
               uncompressed.GetData(),
               uncompressed.GetSize(),
               buffer.GetDataWriteable(),
               buffer.GetSize()
            );
         return buffer;
      };

   duckdb::DuckDB duckDb;
   duckdb::Connection duckdb_connection(duckDb);

   duckdb_connection.CreateScalarFunction<duckdb::string_t, duckdb::string_t, duckdb::string_t>(
      "compressGene",
      zstdCompressOneGenome.target<duckdb::string_t(duckdb::string_t, duckdb::string_t)>()
   );

   executeQuery(
      duckdb_connection, ::fmt::format("SELECT metadata.* FROM 'sample.ndjson.zst';", file_name)
   );

   executeQuery(
      duckdb_connection,
      ::fmt::format(
         "SELECT alignedNucleotideSequences.* FROM 'sample.ndjson.zst' LIMIT 1;", file_name
      )
   );
}
