#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include <duckdb.hpp>

#include "silo/preprocessing/partition.h"
#include "silo/preprocessing/sql_function.h"
#include "silo/storage/reference_genomes.h"
#include "silo/zstd/zstd_compressor.h"

namespace silo::preprocessing {

class PreprocessingDatabase {
  public:
   std::vector<std::unique_ptr<CompressSequence>> compress_nucleotide_functions;
   std::vector<std::unique_ptr<CompressSequence>> compress_amino_acid_functions;

  private:
   duckdb::DuckDB duck_db;
   duckdb::Connection connection;

  public:
   PreprocessingDatabase(
      const std::optional<std::filesystem::path>& backing_file,
      const ReferenceGenomes& reference_genomes,
      std::optional<uint32_t> memory_limit
   );

   duckdb::Connection& getConnection();

   void refreshConnection();

   Partitions getPartitionDescriptor();

   std::unique_ptr<duckdb::MaterializedQueryResult> query(std::string sql_query);
};

std::vector<std::string> extractStringListValue(
   duckdb::MaterializedQueryResult& result,
   size_t row,
   size_t column
);

}  // namespace silo::preprocessing