#include "silo/preprocessing/preprocessing_database.h"

#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <spdlog/spdlog.h>
#include <duckdb.hpp>

#include "silo/common/string_utils.h"
#include "silo/preprocessing/identifiers.h"
#include "silo/preprocessing/partition.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/preprocessing/sql_function.h"
#include "silo/sequence_file_reader/fasta_reader.h"
#include "silo/sequence_file_reader/sam_reader.h"
#include "silo/storage/reference_genomes.h"
#include "silo/zstd/zstd_compressor.h"
#include "silo/zstd/zstd_table.h"

using duckdb::BigIntValue;
using duckdb::ListValue;
using duckdb::MaterializedQueryResult;
using duckdb::Value;

namespace {
constexpr std::string_view FASTA_EXTENSION = "fasta";
constexpr std::string_view SAM_EXTENSION = "sam";
}  // namespace

namespace silo::preprocessing {

PreprocessingDatabase::PreprocessingDatabase(
   const std::optional<std::filesystem::path>& backing_file,
   const ReferenceGenomes& reference_genomes,
   std::optional<uint32_t> memory_limit
)
    : duck_db(backing_file.value_or(":memory:")),
      connection(duck_db) {
   query("PRAGMA default_null_order='NULLS FIRST';");
   query("SET preserve_insertion_order=FALSE;");
   query("SET partitioned_write_flush_threshold = 1024;");
   if (memory_limit.has_value()) {
      query(fmt::format("SET memory_limit='{} GB';", memory_limit.value()));
   }
   const Identifiers compress_nucleotide_function_identifiers =
      Identifiers{reference_genomes.getSequenceNames<Nucleotide>()}.prefix("compress_nuc_");
   const Identifiers compress_amino_acid_function_identifiers =
      Identifiers{reference_genomes.getSequenceNames<AminoAcid>()}.prefix("compress_aa_");
   for (size_t sequence_idx = 0; sequence_idx < reference_genomes.nucleotide_sequence_names.size();
        ++sequence_idx) {
      const auto& reference = reference_genomes.raw_nucleotide_sequences.at(sequence_idx);
      compress_nucleotide_functions.emplace_back(std::make_unique<CompressSequence>(
         compress_nucleotide_function_identifiers.getIdentifier(sequence_idx), reference
      ));
      compress_nucleotide_functions[sequence_idx]->addToConnection(connection);
   }
   for (size_t sequence_idx = 0; sequence_idx < reference_genomes.aa_sequence_names.size();
        ++sequence_idx) {
      const auto& reference = reference_genomes.raw_aa_sequences.at(sequence_idx);
      compress_amino_acid_functions.emplace_back(std::make_unique<CompressSequence>(
         compress_amino_acid_function_identifiers.getIdentifier(sequence_idx), reference
      ));
      compress_amino_acid_functions[sequence_idx]->addToConnection(connection);
   }
}

std::unique_ptr<MaterializedQueryResult> PreprocessingDatabase::query(std::string sql_query) {
   SPDLOG_DEBUG("Preprocessing Database - Query:\n{}", sql_query);
   auto result = connection.Query(sql_query);
   SPDLOG_DEBUG("Preprocessing Database - Result:\n{}", result->ToString());
   if (result->HasError()) {
      throw silo::preprocessing::PreprocessingException(result->GetError());
   }
   return result;
}

duckdb::Connection& PreprocessingDatabase::getConnection() {
   return connection;
}

void PreprocessingDatabase::refreshConnection() {
   connection = duckdb::Connection{duck_db};
}

preprocessing::Partitions PreprocessingDatabase::getPartitionDescriptor() {
   auto partition_descriptor_from_sql =
      connection.Query("SELECT partition_id, count FROM partitioning ORDER BY partition_id");

   std::vector<preprocessing::Partition> partitions;

   uint32_t check_partition_id_sorted_and_contiguous = 0;

   for (auto it = partition_descriptor_from_sql->begin();
        it != partition_descriptor_from_sql->end();
        ++it) {
      const auto db_partition_id = it.current_row.GetValue<Value>(0);
      const uint32_t partition_id = BigIntValue::Get(db_partition_id);
      if (partition_id != check_partition_id_sorted_and_contiguous) {
         throw PreprocessingException(
            "The partition IDs produced by the preprocessing are not sorted, not starting from "
            "0 or not contiguous."
         );
      }
      check_partition_id_sorted_and_contiguous++;

      const auto db_partition_size = it.current_row.GetValue<Value>(1);
      const int64_t partition_size_bigint = BigIntValue::Get(db_partition_size);
      if (partition_size_bigint < 0) {
         throw PreprocessingException("Negative partition size encountered.");
      }
      if (partition_size_bigint > UINT32_MAX) {
         throw PreprocessingException(
            fmt::format("Overflow of limit UINT32_MAX ({}) for number of sequences.", UINT32_MAX)
         );
      }

      const auto partition_size = static_cast<uint32_t>(partition_size_bigint);

      partitions.emplace_back(std::vector<preprocessing::PartitionChunk>{
         {.partition = partition_id, .chunk = 0, .size = partition_size, .offset = 0}
      });
   }

   return preprocessing::Partitions(partitions);
}

ZstdTable PreprocessingDatabase::generateSequenceTableViaFile(
   const std::string& table_name,
   const std::string& reference_sequence,
   const std::filesystem::path& file_path
) {
   const auto file_stem = file_path.stem().string();
   for (const auto& entry : std::filesystem::directory_iterator(file_path.parent_path())) {
      const auto entry_file_name = entry.path().filename().string();
      if (!entry.is_regular_file() || !entry_file_name.starts_with(file_stem)) {
         continue;
      }
      auto extensions = splitBy(entry_file_name, ".");
      auto last = extensions.back();
      if (last == "zst" || last == "xz") {
         extensions.pop_back();
         last = extensions.back();
      }
      if (last == FASTA_EXTENSION) {
         return generateSequenceTableFromFasta(table_name, reference_sequence, entry.path());
      }
      if (last == SAM_EXTENSION) {
         return generateSequenceTableFromSAM(table_name, reference_sequence, entry.path());
      }
   }

   throw PreprocessingException(fmt::format(
      "Could not find reference file for {}, tried file extensions: .fasta(.zst,.xz), "
      ".sam(.zst,.xz)",
      file_path.string()
   ));
}

ZstdTable PreprocessingDatabase::generateSequenceTableFromFasta(
   const std::string& table_name,
   const std::string& reference_sequence,
   const std::filesystem::path& file_name
) {
   silo::sequence_file_reader::FastaReader fasta_reader(file_name);
   return ZstdTable::generate(connection, table_name, fasta_reader, reference_sequence);
}

ZstdTable PreprocessingDatabase::generateSequenceTableFromSAM(
   const std::string& table_name,
   const std::string& reference_sequence,
   const std::filesystem::path& file_name
) {
   silo::sequence_file_reader::SamReader sam_reader(file_name);

   return ZstdTable::generate(connection, table_name, sam_reader, reference_sequence);
}

std::vector<std::string> extractStringListValue(
   MaterializedQueryResult& result,
   size_t row,
   size_t column
) {
   std::vector<std::string> return_value;
   const Value tmp_value = result.GetValue(column, row);
   std::vector<Value> child_values = ListValue::GetChildren(tmp_value);
   std::ranges::transform(child_values, std::back_inserter(return_value), [](const Value& value) {
      return value.GetValue<std::string>();
   });
   return return_value;
}

}  // namespace silo::preprocessing
