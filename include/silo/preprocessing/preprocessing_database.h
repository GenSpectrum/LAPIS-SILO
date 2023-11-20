#pragma once

#include <memory>
#include <string>

#include "duckdb.hpp"

namespace silo {

class ReferenceGenomes;

namespace preprocessing {

class Partitions;

class PreprocessingDatabase {
   duckdb::DuckDB duck_db;
   duckdb::Connection connection;

  public:
   PreprocessingDatabase(const std::string& backing_file);

   duckdb::Connection& getConnection();

   Partitions getPartitionDescriptor();

   void registerSequences(const silo::ReferenceGenomes& reference_genomes);

   std::unique_ptr<duckdb::MaterializedQueryResult> query(std::string sql_query);

   void generateNucSequenceTable(
      const std::string& table_name,
      const std::string& reference_sequence,
      const std::string& filename
   );
};

}  // namespace preprocessing
}  // namespace silo