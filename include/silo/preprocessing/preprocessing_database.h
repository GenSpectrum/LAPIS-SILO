#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include <duckdb.hpp>

namespace silo {

class ZstdFastaTable;
class ReferenceGenomes;
class CompressSequence;

namespace preprocessing {

class Partitions;

class PreprocessingDatabase {
  public:
   std::unordered_map<std::string_view, std::unique_ptr<CompressSequence>>
      compress_nucleotide_functions;
   std::unordered_map<std::string_view, std::unique_ptr<CompressSequence>>
      compress_amino_acid_functions;

  private:
   duckdb::DuckDB duck_db;
   duckdb::Connection connection;

  public:
   PreprocessingDatabase(
      const std::optional<std::filesystem::path>& backing_file,
      const ReferenceGenomes& reference_genomes
   );

   duckdb::Connection& getConnection();

   void refreshConnection();

   Partitions getPartitionDescriptor();

   std::unique_ptr<duckdb::MaterializedQueryResult> query(std::string sql_query);

   ZstdFastaTable generateSequenceTableFromFasta(
      const std::string& table_name,
      const std::string& reference_sequence,
      const std::string& filename
   );

   ZstdFastaTable generateSequenceTableFromZstdFasta(
      const std::string& table_name,
      const std::string& reference_sequence,
      const std::string& filename
   );
};

std::vector<std::string> extractStringListValue(
   duckdb::MaterializedQueryResult& result,
   size_t row,
   size_t column
);

}  // namespace preprocessing
}  // namespace silo