#include "silo/preprocessing/preprocessing_database.h"

#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <spdlog/spdlog.h>
#include <duckdb.hpp>

#include "silo/common/fasta_reader.h"
#include "silo/preprocessing/partition.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/preprocessing/sql_function.h"
#include "silo/storage/reference_genomes.h"
#include "silo/zstdfasta/zstd_compressor.h"
#include "silo/zstdfasta/zstdfasta_reader.h"
#include "silo/zstdfasta/zstdfasta_table.h"
#include "silo/zstdfasta/zstdfasta_writer.h"

using duckdb::BigIntValue;
using duckdb::ListValue;
using duckdb::MaterializedQueryResult;
using duckdb::Value;

namespace silo::preprocessing {

PreprocessingDatabase::PreprocessingDatabase(
   const std::optional<std::filesystem::path>& backing_file,
   const ReferenceGenomes& reference_genomes
)
    : duck_db(backing_file.value_or(":memory:")),
      connection(duck_db) {
   query("PRAGMA default_null_order='NULLS FIRST';");
   query("SET preserve_insertion_order=FALSE;");
   query("SET memory_limit='50 GB';");
   for (const auto& [sequence_name, reference] : reference_genomes.raw_nucleotide_sequences) {
      compress_nucleotide_functions.emplace(
         sequence_name, std::make_unique<CompressSequence>("nuc", sequence_name, reference)
      );
      compress_nucleotide_functions[sequence_name]->addToConnection(connection);
   }
   for (const auto& [sequence_name, reference] : reference_genomes.raw_aa_sequences) {
      compress_amino_acid_functions.emplace(
         sequence_name, std::make_unique<CompressSequence>("aa", sequence_name, reference)
      );
      compress_amino_acid_functions[sequence_name]->addToConnection(connection);
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

ZstdFastaTable PreprocessingDatabase::generateSequenceTableFromFasta(
   const std::string& table_name,
   const std::string& reference_sequence,
   const std::string& filename
) {
   silo::FastaReader fasta_reader(filename);
   return ZstdFastaTable::generate(connection, table_name, fasta_reader, reference_sequence);
}

ZstdFastaTable PreprocessingDatabase::generateSequenceTableFromZstdFasta(
   const std::string& table_name,
   const std::string& reference_sequence,
   const std::string& filename
) {
   silo::ZstdFastaReader zstd_fasta_reader(filename, reference_sequence);
   return ZstdFastaTable::generate(connection, table_name, zstd_fasta_reader, reference_sequence);
}

std::vector<std::string> extractStringListValue(
   MaterializedQueryResult& result,
   size_t row,
   size_t column
) {
   std::vector<std::string> return_value;
   const Value tmp_value = result.GetValue(column, row);
   std::vector<Value> child_values = ListValue::GetChildren(tmp_value);
   std::transform(
      child_values.begin(),
      child_values.end(),
      std::back_inserter(return_value),
      [](const Value& value) { return value.GetValue<std::string>(); }
   );
   return return_value;
}

}  // namespace silo::preprocessing
