#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include <duckdb.hpp>

namespace silo {

class ZstdTable;
class ReferenceGenomes;
class CompressSequence;

namespace preprocessing {

class Partitions;

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
      const ReferenceGenomes& reference_genomes
   );

   duckdb::Connection& getConnection();

   void refreshConnection();

   Partitions getPartitionDescriptor();

   std::unique_ptr<duckdb::MaterializedQueryResult> query(std::string sql_query);

   ZstdTable generateSequenceTableViaFile(
      const std::string& table_name,
      const std::string& reference_sequence,
      const std::filesystem::path& file_path
   );

   ZstdTable generateSequenceTableFromFasta(
      const std::string& table_name,
      const std::string& reference_sequence,
      const std::filesystem::path& file_name
   );

   ZstdTable generateSequenceTableFromSAM(
      const std::string& table_name,
      const std::string& reference_sequence,
      const std::filesystem::path& file_name
   );
};

std::vector<std::string> extractStringListValue(
   duckdb::MaterializedQueryResult& result,
   size_t row,
   size_t column
);

}  // namespace preprocessing
}  // namespace silo