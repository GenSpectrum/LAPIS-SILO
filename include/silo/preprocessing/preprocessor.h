#pragma once

#include "silo/config/database_config.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_database.h"

namespace silo {
class Database;
class PangoLineageAliasLookup;

namespace preprocessing {

class Preprocessor {
   PreprocessingConfig preprocessing_config;
   config::DatabaseConfig database_config;
   PreprocessingDatabase preprocessing_db;

  public:
   Preprocessor(
      const preprocessing::PreprocessingConfig& preprocessing_config,
      const config::DatabaseConfig& database_config
   );

   Database preprocess();

  private:
   void buildTablesFromInput(const ReferenceGenomes& reference_genomes);

   Database buildDatabase(
      const preprocessing::Partitions& partition_descriptor,
      const ReferenceGenomes& reference_genomes,
      const std::string& order_by_clause,
      const silo::PangoLineageAliasLookup& alias_key
   );
};
}  // namespace preprocessing
}  // namespace silo
