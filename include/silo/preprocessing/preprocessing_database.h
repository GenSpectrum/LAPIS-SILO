#pragma once

#include <memory>
#include <string>

#include "duckdb.hpp"

namespace silo {

class ReferenceGenomes;

namespace preprocessing {

class Partitions;

class PreprocessingDatabase {
  public:
   static constexpr std::string_view COMPRESS_NUC = "compressNuc";
   static constexpr std::string_view COMPRESS_AA = "compressAA";

  private:
   duckdb::DuckDB duck_db;
   duckdb::Connection connection;

  public:
   PreprocessingDatabase(const std::string& backing_file);

   duckdb::Connection& getConnection();

   Partitions getPartitionDescriptor();

   static void registerSequences(const silo::ReferenceGenomes& reference_genomes);

   std::unique_ptr<duckdb::MaterializedQueryResult> query(std::string sql_query);

   void generateNucSequenceTable(
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