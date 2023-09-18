#include "silo/preprocessing/ndjson_digestion.h"

#include <fmt/format.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>
#include <duckdb.hpp>

#include "silo/common/zstd_compressor.h"
#include "silo/common/zstd_decompressor.h"
#include "silo/common/zstdfasta_writer.h"
#include "silo/storage/reference_genomes.h"

void executeQuery(duckdb::Connection& db, std::string sql_query) {
   SPDLOG_INFO("executing duckdb query: {}", sql_query);
   auto res = db.Query(sql_query);
   SPDLOG_INFO("duckdb Result: {}", res->ToString());
}

class Compressors {
  public:
   static std::
      unordered_map<std::string_view, tbb::enumerable_thread_specific<silo::ZstdCompressor>>
         nuc_compressors;
   static std::unordered_map<std::string_view, tbb::enumerable_thread_specific<std::string>>
      nuc_buffers;
   static std::
      unordered_map<std::string_view, tbb::enumerable_thread_specific<silo::ZstdCompressor>>
         aa_compressors;
   static std::unordered_map<std::string_view, tbb::enumerable_thread_specific<std::string>>
      aa_buffers;

   static void initialize(const silo::ReferenceGenomes& reference_genomes) {
      for (const auto& [name, sequence] : reference_genomes.raw_nucleotide_sequences) {
         silo::ZstdCompressor exemplar(sequence);
         nuc_buffers.emplace(name, std::string(exemplar.getSizeBound(), '\0'));
         nuc_compressors.emplace(name, std::move(exemplar));
      }
      for (const auto& [name, sequence] : reference_genomes.raw_aa_sequences) {
         silo::ZstdCompressor exemplar(sequence);
         aa_buffers.emplace(name, std::string(exemplar.getSizeBound(), '\0'));
         aa_compressors.emplace(name, std::move(exemplar));
      }
   }

   static duckdb::string_t compressNuc(
      const duckdb::string_t uncompressed,
      duckdb::string_t genome_name
   ) {
      std::string& buffer = nuc_buffers.at(genome_name.GetString()).local();
      size_t size_or_error_code =
         nuc_compressors.at(genome_name.GetString())
            .local()
            .compress(uncompressed.GetData(), uncompressed.GetSize(), buffer.data(), buffer.size());
      // TODO check size_or_error_code
      auto result = new std::string(buffer.data(), size_or_error_code);
      return duckdb::string_t{result->data(), static_cast<uint32_t>(size_or_error_code)};
   };

   static duckdb::string_t compressAA(
      const duckdb::string_t uncompressed,
      duckdb::string_t gene_name
   ) {
      std::string& buffer = aa_buffers.at(gene_name.GetString()).local();
      size_t size_or_error_code =
         aa_compressors.at(gene_name.GetString())
            .local()
            .compress(uncompressed.GetData(), uncompressed.GetSize(), buffer.data(), buffer.size());
      // TODO check size_or_error_code
      auto result = new std::string(buffer.data(), size_or_error_code);
      return duckdb::string_t{result->data(), static_cast<uint32_t>(size_or_error_code)};
   };
};

std::unordered_map<std::string_view, tbb::enumerable_thread_specific<silo::ZstdCompressor>>
   Compressors::nuc_compressors{};
std::unordered_map<std::string_view, tbb::enumerable_thread_specific<std::string>>
   Compressors::nuc_buffers{};
std::unordered_map<std::string_view, tbb::enumerable_thread_specific<silo::ZstdCompressor>>
   Compressors::aa_compressors{};
std::unordered_map<std::string_view, tbb::enumerable_thread_specific<std::string>>
   Compressors::aa_buffers{};

class Decompressors {
  private:
   static std::unordered_map<std::string_view, silo::ZstdDecompressor> nuc_decompressors;
   static std::unordered_map<std::string_view, size_t> nuc_reference_lengths;
   static std::unordered_map<std::string_view, silo::ZstdDecompressor> aa_decompressors;
   static std::unordered_map<std::string_view, size_t> aa_reference_lengths;

  public:
   static void initialize(const silo::ReferenceGenomes& reference_genomes) {
      for (const auto& [name, sequence] : reference_genomes.raw_nucleotide_sequences) {
         nuc_decompressors.emplace(name, silo::ZstdDecompressor(sequence));
         nuc_reference_lengths.emplace(name, sequence.size());
      }
      for (const auto& [name, sequence] : reference_genomes.raw_aa_sequences) {
         aa_decompressors.emplace(name, silo::ZstdDecompressor(sequence));
         aa_reference_lengths.emplace(name, sequence.size());
      }
   }

   static duckdb::string_t decompressNuc(
      const duckdb::string_t uncompressed,
      duckdb::string_t genome_name
   ) {
      std::string* result =
         new std::string(nuc_reference_lengths.at(genome_name.GetString()), '\0');
      nuc_decompressors.at(genome_name.GetString())
         .decompress(
            uncompressed.GetData(), uncompressed.GetSize(), result->data(), result->size()
         );
      return duckdb::string_t{result->data(), static_cast<uint32_t>(result->size())};
   };

   static duckdb::string_t decompressAA(
      const duckdb::string_t uncompressed,
      duckdb::string_t gene_name
   ) {
      std::string* result = new std::string(aa_reference_lengths.at(gene_name.GetString()), '\0');
      aa_decompressors.at(gene_name.GetString())
         .decompress(
            uncompressed.GetData(), uncompressed.GetSize(), result->data(), result->size()
         );
      return duckdb::string_t{result->data(), static_cast<uint32_t>(result->size())};
   };
};

std::unordered_map<std::string_view, silo::ZstdDecompressor> Decompressors::nuc_decompressors{};
std::unordered_map<std::string_view, size_t> Decompressors::nuc_reference_lengths{};
std::unordered_map<std::string_view, silo::ZstdDecompressor> Decompressors::aa_decompressors{};
std::unordered_map<std::string_view, size_t> Decompressors::aa_reference_lengths{};

std::vector<std::string> extractStringListValue(
   duckdb::MaterializedQueryResult& result,
   size_t row,
   size_t column
) {
   std::vector<std::string> return_value;
   duckdb::Value tmp_value = result.GetValue(column, row);
   std::vector<duckdb::Value> child_values = duckdb::ListValue::GetChildren(tmp_value);
   std::transform(
      child_values.begin(),
      child_values.end(),
      std::back_inserter(return_value),
      [](const duckdb::Value& value) { return value.GetValue<std::string>(); }
   );
   return return_value;
}

void silo::executeDuckDBRoutineForNdjsonDigestion(
   const silo::Database& database,
   const silo::ReferenceGenomes& reference_genomes,
   std::string_view file_name
) {
   file_name = "sample.ndjson.zst";  // TODO remove

   Compressors::initialize(reference_genomes);
   Decompressors::initialize(reference_genomes);

   duckdb::DuckDB duckDb;
   duckdb::Connection duckdb_connection(duckDb);

   duckdb_connection.CreateScalarFunction<duckdb::string_t, duckdb::string_t, duckdb::string_t>(
      "compressNuc",
      {duckdb::LogicalType::VARCHAR, duckdb::LogicalType::VARCHAR},
      duckdb::LogicalType::BLOB,
      Compressors::compressNuc
   );

   duckdb_connection.CreateScalarFunction<duckdb::string_t, duckdb::string_t, duckdb::string_t>(
      "decompressNuc",
      {duckdb::LogicalType::BLOB, duckdb::LogicalType::VARCHAR},
      duckdb::LogicalType::VARCHAR,
      Decompressors::decompressNuc
   );

   duckdb_connection.CreateScalarFunction<duckdb::string_t, duckdb::string_t, duckdb::string_t>(
      "compressAA",
      {duckdb::LogicalType::VARCHAR, duckdb::LogicalType::VARCHAR},
      duckdb::LogicalType::BLOB,
      Compressors::compressAA
   );

   duckdb_connection.CreateScalarFunction<duckdb::string_t, duckdb::string_t, duckdb::string_t>(
      "decompressAA",
      {duckdb::LogicalType::BLOB, duckdb::LogicalType::VARCHAR},
      duckdb::LogicalType::VARCHAR,
      Decompressors::decompressAA
   );

   auto res = duckdb_connection.Query(fmt::format(
      "SELECT json_keys(alignedNucleotideSequences), json_keys(alignedAminoAcidSequences) FROM "
      "'{}' LIMIT 1; ",
      file_name
   ));
   if (res->HasError() || res->RowCount() != 1) {
      throw std::runtime_error("Preprocessing exception " + std::to_string(res->RowCount()));
   }

   std::vector<std::string> nuc_sequence_names = extractStringListValue(*res, 0, 0);
   std::vector<std::string> aa_sequence_names = extractStringListValue(*res, 0, 1);

   std::vector<std::string> sequence_selects;
   for (const std::string& name : nuc_sequence_names) {
      if (!reference_genomes.raw_nucleotide_sequences.contains(name)) {
         throw std::runtime_error("TMP");  // TODO throw appropriate error
      }
      sequence_selects.emplace_back(fmt::format(
         "compressNuc(alignedNucleotideSequences.{0}, "
         "'{0}') as nuc_{0}",
         name
      ));
   }
   for (const std::string& name : aa_sequence_names) {
      if (!reference_genomes.raw_aa_sequences.contains(name)) {
         throw std::runtime_error("TMP");  // TODO throw appropriate error
      }
      sequence_selects.emplace_back(fmt::format(
         "compressAA(alignedAminoAcidSequences.{0}, "
         "'{0}') as gene_{0}",
         name
      ));
   }

   executeQuery(
      duckdb_connection,
      ::fmt::format(
         "CREATE TABLE preprocessing_table AS SELECT metadata, {} FROM '{}'; ",
         boost::join(sequence_selects, ","),
         file_name
      )
   );

   executeQuery(duckdb_connection, "SELECT * FROM preprocessing_table;");
   executeQuery(
      duckdb_connection,
      "COPY (SELECT metadata.* FROM preprocessing_table) TO 'metadata.tsv' WITH "
      "(HEADER 1, "
      "DELIMITER '\t');"
   );
   for (const std::string& nuc_sequence_name : nuc_sequence_names) {
      res = duckdb_connection.Query(fmt::format(
         "SELECT metadata.genbankAccession, nuc_{} FROM preprocessing_table", nuc_sequence_name
      ));
      ZstdFastaWriter writer(
         "./nuc_" + nuc_sequence_name + ".zstdfasta",
         reference_genomes.raw_nucleotide_sequences.at(nuc_sequence_name)
      );
      for (auto it = res->begin(); it != res->end(); ++it) {
         const duckdb::Value& primary_key = it.current_row.GetValue<duckdb::Value>(0);
         const duckdb::Value& sequence_blob = it.current_row.GetValue<duckdb::Value>(1);
         writer.writeRaw(
            duckdb::StringValue::Get(primary_key), duckdb::StringValue::Get(sequence_blob)
         );
      }
   }
   for (const std::string& aa_sequence_name : aa_sequence_names) {
      res = duckdb_connection.Query(fmt::format(
         "SELECT metadata.genbankAccession, gene_{} FROM preprocessing_table", aa_sequence_name
      ));
      ZstdFastaWriter writer(
         "./gene_" + aa_sequence_name + ".zstdfasta",
         reference_genomes.raw_aa_sequences.at(aa_sequence_name)
      );
      for (auto it = res->begin(); it != res->end(); ++it) {
         const duckdb::Value& primary_key = it.current_row.GetValue<duckdb::Value>(0);
         const duckdb::Value& sequence_blob = it.current_row.GetValue<duckdb::Value>(1);
         writer.writeRaw(
            duckdb::StringValue::Get(primary_key), duckdb::StringValue::Get(sequence_blob)
         );
      }
   }
}
