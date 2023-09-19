#include "silo/preprocessing/ndjson_digestion.h"

#include <fmt/format.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>
#include <duckdb.hpp>

#include "silo/common/zstd_compressor.h"
#include "silo/common/zstd_decompressor.h"
#include "silo/common/zstdfasta_writer.h"
#include "silo/preprocessing/preprocessing_exception.h"
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
   static tbb::enumerable_thread_specific<std::deque<std::string>> sequence_heaps;

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
      auto& result = sequence_heaps.local().emplace_back(buffer.data(), size_or_error_code);
      return duckdb::string_t{result.data(), static_cast<uint32_t>(size_or_error_code)};
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
      auto& result = sequence_heaps.local().emplace_back(buffer.data(), size_or_error_code);
      return duckdb::string_t{result.data(), static_cast<uint32_t>(size_or_error_code)};
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
tbb::enumerable_thread_specific<std::deque<std::string>> Compressors::sequence_heaps{};

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

void exportMetadataFile(duckdb::Connection& duckdb_connection) {
   executeQuery(
      duckdb_connection,
      "COPY (SELECT metadata.* FROM preprocessing_table) TO 'metadata.tsv' WITH "
      "(HEADER 1, "
      "DELIMITER '\t');"
   );
}

void exportSequenceFiles(
   duckdb::Connection& duckdb_connection,
   const std::vector<std::string>& nuc_sequence_names,
   const std::vector<std::string>& aa_sequence_names,
   const silo::ReferenceGenomes& reference_genomes
) {
   for (const std::string& nuc_sequence_name : nuc_sequence_names) {
      SPDLOG_INFO("Writing zstdfasta file for nucleotide sequence: {}", nuc_sequence_name);

      const std::string query = fmt::format(
         "SELECT metadata.strain, nuc_{} FROM preprocessing_table", nuc_sequence_name
      );  // TODO format with primary key

      SPDLOG_DEBUG("Executing query: {}", query);

      auto result = duckdb_connection.Query(query);

      SPDLOG_DEBUG("Result size: {}", result->RowCount());

      silo::ZstdFastaWriter writer(
         "./nuc_" + nuc_sequence_name + ".zstdfasta",
         reference_genomes.raw_nucleotide_sequences.at(nuc_sequence_name)
      );

      for (auto it = result->begin(); it != result->end(); ++it) {
         const duckdb::Value& primary_key = it.current_row.GetValue<duckdb::Value>(0);
         if (!primary_key.IsNull()) {  // TODO remove with cleaned dataset. Warn about it instead
            const duckdb::Value& sequence_blob = it.current_row.GetValue<duckdb::Value>(1);
            writer.writeRaw(
               duckdb::StringValue::Get(primary_key), duckdb::StringValue::Get(sequence_blob)
            );
         }
      }
   }
   for (const std::string& aa_sequence_name : aa_sequence_names) {
      SPDLOG_INFO("Writing zstdfasta file for amino acid sequence: {}", aa_sequence_name);

      const std::string query = fmt::format(
         "SELECT metadata.strain, gene_{} FROM preprocessing_table", aa_sequence_name
      );  // TODO format with primary key

      SPDLOG_DEBUG("Executing query: {}", query);

      auto result = duckdb_connection.Query(query);

      SPDLOG_DEBUG("Result size: {}", result->RowCount());

      silo::ZstdFastaWriter writer(
         "./gene_" + aa_sequence_name + ".zstdfasta",
         reference_genomes.raw_aa_sequences.at(aa_sequence_name)
      );
      for (auto it = result->begin(); it != result->end(); ++it) {
         const duckdb::Value& primary_key = it.current_row.GetValue<duckdb::Value>(0);
         if (!primary_key.IsNull()) {  // TODO remove with cleaned dataset. Warn about it instead
            const duckdb::Value& sequence_blob = it.current_row.GetValue<duckdb::Value>(1);
            writer.writeRaw(
               duckdb::StringValue::Get(primary_key), duckdb::StringValue::Get(sequence_blob)
            );
         }
      }
   }
}

void silo::executeDuckDBRoutineForNdjsonDigestion(
   const silo::Database& database,
   const silo::ReferenceGenomes& reference_genomes,
   std::string_view file_name
) {
   Compressors::initialize(reference_genomes);

   duckdb::DuckDB duckDb;
   duckdb::Connection duckdb_connection(duckDb);

   duckdb_connection.CreateScalarFunction<duckdb::string_t, duckdb::string_t, duckdb::string_t>(
      "compressNuc",
      {duckdb::LogicalType::VARCHAR, duckdb::LogicalType::VARCHAR},
      duckdb::LogicalType::BLOB,
      Compressors::compressNuc
   );

   duckdb_connection.CreateScalarFunction<duckdb::string_t, duckdb::string_t, duckdb::string_t>(
      "compressAA",
      {duckdb::LogicalType::VARCHAR, duckdb::LogicalType::VARCHAR},
      duckdb::LogicalType::BLOB,
      Compressors::compressAA
   );

   auto res = duckdb_connection.Query(fmt::format(
      "SELECT json_keys(alignedNucleotideSequences), json_keys(alignedAminoAcidSequences) FROM "
      "'{}' LIMIT 1; ",
      file_name
   ));
   if (res->HasError() || res->RowCount() != 1) {
      throw silo::PreprocessingException(
         "Preprocessing exception " + std::to_string(res->RowCount())
      );
   }

   std::vector<std::string> nuc_sequence_names = extractStringListValue(*res, 0, 0);
   std::vector<std::string> aa_sequence_names = extractStringListValue(*res, 0, 1);

   std::vector<std::string> sequence_selects;
   for (const std::string& name : nuc_sequence_names) {
      if (!reference_genomes.raw_nucleotide_sequences.contains(name)) {
         throw silo::PreprocessingException(fmt::format(
            "The aligned nucleotide sequence {} which is contained in the input file {} is not "
            "contained in the reference sequences.",
            name,
            file_name
         ));
      }
      sequence_selects.emplace_back(fmt::format(
         "compressNuc(alignedNucleotideSequences.{0}, "
         "'{0}') as nuc_{0}",
         name
      ));
   }
   for (const std::string& name : aa_sequence_names) {
      if (!reference_genomes.raw_aa_sequences.contains(name)) {
         throw silo::PreprocessingException(fmt::format(
            "The aligned amino acid sequence {} which is contained in the input file {} is not "
            "contained in the reference sequences.",
            name,
            file_name
         ));
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

   exportMetadataFile(duckdb_connection);

   exportSequenceFiles(duckdb_connection, nuc_sequence_names, aa_sequence_names, reference_genomes);

   for (auto& sequence_heap : Compressors::sequence_heaps) {
      sequence_heap.clear();
   }
}
