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
#include "silo/storage/reference_genomes.h"
#include "silo/zstdfasta/zstd_compressor.h"
#include "silo/zstdfasta/zstdfasta_reader.h"
#include "silo/zstdfasta/zstdfasta_table.h"
#include "silo/zstdfasta/zstdfasta_writer.h"

using duckdb::BigIntValue;
using duckdb::BinaryExecutor;
using duckdb::Connection;
using duckdb::DataChunk;
using duckdb::ExpressionState;
using duckdb::ListValue;
using duckdb::LogicalType;
using duckdb::MaterializedQueryResult;
using duckdb::string_t;
using duckdb::StringVector;
using duckdb::Value;
using duckdb::Vector;

namespace {

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

   static void compressNuc(DataChunk& args, ExpressionState& /*state*/, Vector& result) {
      BinaryExecutor::Execute<string_t, string_t, string_t>(
         args.data[0],
         args.data[1],
         result,
         args.size(),
         [&](const string_t uncompressed, const string_t segment_name) {
            std::string& buffer = nuc_buffers.at(segment_name.GetString()).local();
            const size_t size_or_error_code =
               nuc_compressors.at(segment_name.GetString())
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

   static void compressAA(DataChunk& args, ExpressionState& /*state*/, Vector& result) {
      BinaryExecutor::Execute<string_t, string_t, string_t>(
         args.data[0],
         args.data[1],
         result,
         args.size(),
         [&](const string_t uncompressed, const string_t gene_name) {
            std::string& buffer = aa_buffers.at(gene_name.GetString()).local();
            const size_t size_or_error_code =
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

}  // namespace

namespace silo::preprocessing {

PreprocessingDatabase::PreprocessingDatabase(const std::string& backing_file)
    : duck_db(backing_file),
      connection(duck_db) {
   auto return_code = connection.Query("PRAGMA default_null_order='NULLS FIRST';");
   if (return_code->HasError()) {
      throw PreprocessingException(return_code->GetError());
   }

   connection.CreateVectorizedFunction(
      std::string(COMPRESS_NUC),
      {LogicalType::VARCHAR, LogicalType::VARCHAR},
      LogicalType::BLOB,
      Compressors::compressNuc
   );

   connection.CreateVectorizedFunction(
      std::string(COMPRESS_AA),
      {LogicalType::VARCHAR, LogicalType::VARCHAR},
      LogicalType::BLOB,
      Compressors::compressAA
   );
}

std::unique_ptr<MaterializedQueryResult> PreprocessingDatabase::query(std::string sql_query) {
   SPDLOG_DEBUG("Preprocessing Database - Query:\n{}", sql_query);
   auto result = connection.Query(sql_query);
   SPDLOG_DEBUG("Preprocessing Database - Result:\n{}", result->ToString());
   if (result->HasError()) {
      throw silo::preprocessing::PreprocessingException(result->ToString());
   }
   return result;
}

void PreprocessingDatabase::registerSequences(const silo::ReferenceGenomes& reference_genomes) {
   Compressors::initialize(reference_genomes);
}

Connection& PreprocessingDatabase::getConnection() {
   return connection;
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
            "The partition IDs produced by the preprocessing are not sorted, not starting from 0 "
            "or not contiguous."
         );
      }
      check_partition_id_sorted_and_contiguous++;

      const auto db_partition_size = it.current_row.GetValue<Value>(1);
      const int64_t partition_size_bigint = BigIntValue::Get(db_partition_size);
      if (partition_size_bigint <= 0) {
         throw PreprocessingException("Non-positive partition size encountered.");
      }
      if (partition_size_bigint > UINT32_MAX) {
         throw PreprocessingException(
            fmt::format("Overflow of limit UINT32_MAX ({}) for number of sequences.", UINT32_MAX)
         );
      }

      const auto partition_size = static_cast<uint32_t>(partition_size_bigint);

      partitions.emplace_back(
         std::vector<preprocessing::PartitionChunk>{{partition_id, 0, partition_size, 0}}
      );
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
