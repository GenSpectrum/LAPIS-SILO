#include "silo/preprocessing/ndjson_digestion.h"

#include <fmt/format.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>
#include <duckdb.hpp>

#include "silo/common/zstd_compressor.h"
#include "silo/common/zstd_decompressor.h"
#include "silo/common/zstdfasta_writer.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/reference_genomes.h"

std::unique_ptr<duckdb::MaterializedQueryResult> executeQuery(
   duckdb::Connection& db,
   std::string sql_query
) {
   SPDLOG_INFO("executing duckdb query: {}", sql_query);
   auto res = db.Query(sql_query);
   SPDLOG_INFO("duckdb Result: {}", res->ToString());
   return res;
}

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

class SequenceNames {
   std::string input_filename;
   std::vector<std::string> nuc_sequence_names;
   std::vector<std::string> aa_sequence_names;

  public:
   SequenceNames(duckdb::Connection& duckdb_connection, std::string_view input_filename)
       : input_filename(input_filename) {
      auto result = duckdb_connection.Query(fmt::format(
         "SELECT json_keys(alignedNucleotideSequences), json_keys(alignedAminoAcidSequences) "
         "FROM "
         "'{}' LIMIT 1; ",
         input_filename
      ));
      if (result->HasError() || result->RowCount() != 1) {
         throw silo::PreprocessingException(
            "Preprocessing exception " + std::to_string(result->RowCount())
         );
      }

      nuc_sequence_names = extractStringListValue(*result, 0, 0);
      aa_sequence_names = extractStringListValue(*result, 0, 1);
   }

   std::vector<std::string>& getNucSequenceNames() { return this->nuc_sequence_names; }

   std::vector<std::string>& getAASequenceNames() { return this->aa_sequence_names; }

   std::vector<std::string> getSequenceSelects() {
      std::vector<std::string> sequence_selects;
      for (const std::string& name : nuc_sequence_names) {
         sequence_selects.emplace_back(fmt::format(
            "compressNuc(alignedNucleotideSequences.{0}, "
            "'{0}') as nuc_{0}",
            name
         ));
      }
      for (const std::string& name : aa_sequence_names) {
         sequence_selects.emplace_back(fmt::format(
            "compressAA(alignedAminoAcidSequences.{0}, "
            "'{0}') as gene_{0}",
            name
         ));
      }
      return sequence_selects;
   }

   void validate(const silo::ReferenceGenomes& reference_genomes) {
      for (const std::string& name : nuc_sequence_names) {
         if (!reference_genomes.raw_nucleotide_sequences.contains(name)) {
            throw silo::PreprocessingException(fmt::format(
               "The aligned nucleotide sequence {} which is contained in the input file {} is "
               "not "
               "contained in the reference sequences.",
               name,
               input_filename
            ));
         }
      }
      for (const std::string& name : aa_sequence_names) {
         if (!reference_genomes.raw_aa_sequences.contains(name)) {
            throw silo::PreprocessingException(fmt::format(
               "The aligned amino acid sequence {} which is contained in the input file {} is "
               "not "
               "contained in the reference sequences.",
               name,
               input_filename
            ));
         }
      }
   }
};

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

   static void compressNuc(
      duckdb::DataChunk& args,
      duckdb::ExpressionState& /*state*/,
      duckdb::Vector& result
   ) {
      using namespace duckdb;
      BinaryExecutor::Execute<string_t, string_t, string_t>(
         args.data[0],
         args.data[1],
         result,
         args.size(),
         [&](const duckdb::string_t uncompressed, const duckdb::string_t genome_name) {
            std::string& buffer = nuc_buffers.at(genome_name.GetString()).local();
            size_t size_or_error_code =
               nuc_compressors.at(genome_name.GetString())
                  .local()
                  .compress(
                     uncompressed.GetData(), uncompressed.GetSize(), buffer.data(), buffer.size()
                  );
            return StringVector::AddStringOrBlob(
               result, buffer.data(), static_cast<uint32_t>(size_or_error_code)
            );
         }
      );
   };

   static void compressAA(
      duckdb::DataChunk& args,
      duckdb::ExpressionState& /*state*/,
      duckdb::Vector& result
   ) {
      using namespace duckdb;
      BinaryExecutor::Execute<string_t, string_t, string_t>(
         args.data[0],
         args.data[1],
         result,
         args.size(),
         [&](const duckdb::string_t uncompressed, const duckdb::string_t gene_name) {
            std::string& buffer = aa_buffers.at(gene_name.GetString()).local();
            size_t size_or_error_code =
               aa_compressors.at(gene_name.GetString())
                  .local()
                  .compress(
                     uncompressed.GetData(), uncompressed.GetSize(), buffer.data(), buffer.size()
                  );
            return StringVector::AddStringOrBlob(
               result, buffer.data(), static_cast<uint32_t>(size_or_error_code)
            );
         }
      );
   }
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

void exportMetadataFile(
   duckdb::Connection& duckdb_connection,
   const std::filesystem::path& metadata_filename
) {
   executeQuery(
      duckdb_connection,
      fmt::format(
         "COPY (SELECT metadata.* FROM preprocessing_table) TO '{}' WITH (HEADER 1, DELIMITER "
         "'\t');",
         metadata_filename.string()
      )
   );
}

void exportSequenceFiles(
   duckdb::Connection& duckdb_connection,
   SequenceNames sequence_names,
   const silo::ReferenceGenomes& reference_genomes,
   std::string_view primary_key_metadata_column,
   const silo::preprocessing::PreprocessingConfig& preprocessing_config
) {
   for (const std::string& nuc_sequence_name : sequence_names.getNucSequenceNames()) {
      SPDLOG_INFO("Writing zstdfasta file for nucleotide sequence: {}", nuc_sequence_name);

      const std::string query = fmt::format(
         "SELECT metadata.{}, nuc_{} FROM preprocessing_table",
         primary_key_metadata_column,
         nuc_sequence_name
      );

      SPDLOG_DEBUG("Executing query: {}", query);

      auto result = duckdb_connection.Query(query);

      SPDLOG_DEBUG("Result size: {}", result->RowCount());

      silo::ZstdFastaWriter writer(
         preprocessing_config.getNucFilename(nuc_sequence_name),
         reference_genomes.raw_nucleotide_sequences.at(nuc_sequence_name)
      );

      for (auto it = result->begin(); it != result->end(); ++it) {
         const duckdb::Value& primary_key = it.current_row.GetValue<duckdb::Value>(0);
         const duckdb::Value& sequence_blob = it.current_row.GetValue<duckdb::Value>(1);
         if (primary_key.IsNull()) {
            if (primary_key.IsNull()) {
               SPDLOG_WARN(
                  "There is a primary key that is null. Using the empty string for the file "
                  "containing its {} sequence.",
                  nuc_sequence_name
               );
               writer.writeRaw("", duckdb::StringValue::Get(sequence_blob));
               continue;
            }
            continue;
         }
         writer.writeRaw(
            duckdb::StringValue::Get(primary_key), duckdb::StringValue::Get(sequence_blob)
         );
      }
   }
   for (const std::string& aa_sequence_name : sequence_names.getAASequenceNames()) {
      SPDLOG_INFO("Writing zstdfasta file for amino acid sequence: {}", aa_sequence_name);

      const std::string query = fmt::format(
         "SELECT metadata.{}, gene_{} FROM preprocessing_table",
         primary_key_metadata_column,
         aa_sequence_name
      );

      SPDLOG_DEBUG("Executing query: {}", query);

      auto result = duckdb_connection.Query(query);

      SPDLOG_DEBUG("Result size: {}", result->RowCount());

      silo::ZstdFastaWriter writer(
         preprocessing_config.getGeneFilename(aa_sequence_name),
         reference_genomes.raw_aa_sequences.at(aa_sequence_name)
      );
      for (auto it = result->begin(); it != result->end(); ++it) {
         const duckdb::Value& primary_key = it.current_row.GetValue<duckdb::Value>(0);
         const duckdb::Value& sequence_blob = it.current_row.GetValue<duckdb::Value>(1);
         if (primary_key.IsNull()) {
            SPDLOG_WARN(
               "There is a primary key that is null. Using the empty string for the file "
               "containing its {} sequence.",
               aa_sequence_name
            );
            writer.writeRaw("", duckdb::StringValue::Get(sequence_blob));
            continue;
         }
         writer.writeRaw(
            duckdb::StringValue::Get(primary_key), duckdb::StringValue::Get(sequence_blob)
         );
      }
   }
}

void silo::executeDuckDBRoutineForNdjsonDigestion(
   const silo::preprocessing::PreprocessingConfig& preprocessing_config,
   const silo::ReferenceGenomes& reference_genomes,
   std::string_view file_name,
   std::string_view primary_key_metadata_column
) {
   if (!std::filesystem::exists(file_name)) {
      throw silo::PreprocessingException(
         fmt::format("The specified input file {} does not exist.", file_name)
      );
   }
   // TODO validate primary key

   file_name = "sample.ndjson.zst";  // TODO remove

   Compressors::initialize(reference_genomes);

   duckdb::DuckDB duckDb;
   duckdb::Connection duckdb_connection(duckDb);

   duckdb_connection.CreateVectorizedFunction(
      "compressNuc",
      {duckdb::LogicalType::VARCHAR, duckdb::LogicalType::VARCHAR},
      duckdb::LogicalType::BLOB,
      Compressors::compressNuc
   );

   duckdb_connection.CreateVectorizedFunction(
      "compressAA",
      {duckdb::LogicalType::VARCHAR, duckdb::LogicalType::VARCHAR},
      duckdb::LogicalType::BLOB,
      Compressors::compressAA
   );

   SequenceNames sequence_names(duckdb_connection, file_name);

   executeQuery(
      duckdb_connection,
      ::fmt::format(
         "CREATE TABLE preprocessing_table AS SELECT metadata, {} FROM '{}'; ",
         boost::join(sequence_names.getSequenceSelects(), ","),
         file_name
      )
   );

   exportMetadataFile(duckdb_connection, preprocessing_config.getMetadataInputFilename());

   exportSequenceFiles(
      duckdb_connection,
      sequence_names,
      reference_genomes,
      primary_key_metadata_column,
      preprocessing_config
   );

   for (auto& sequence_heap : Compressors::sequence_heaps) {
      sequence_heap.clear();
   }
}
