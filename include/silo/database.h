#ifndef SILO_DATABASE_H
#define SILO_DATABASE_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "silo/config/database_config.h"
#include "silo/preprocessing/pango_lineage_count.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/pango_lineage_alias.h"

namespace silo {

namespace preprocessing {
struct PreprocessingConfig;
struct Partitions;

}  // namespace preprocessing

namespace config {
class DatabaseConfig;
}  // namespace config

struct DatabaseInfo;
struct DetailedDatabaseInfo;
struct BitmapSizePerSymbol;
struct BitmapContainerSize;

class Database {
  public:
   silo::config::DatabaseConfig database_config;
   std::vector<std::string> global_reference;
   std::vector<DatabasePartition> partitions;

   Database();

   explicit Database(const std::string& directory);

   void preprocessing(
      const preprocessing::PreprocessingConfig& preprocessing_config,
      const config::DatabaseConfig& database_config_
   );

   void build(
      const std::string& partition_name_prefix,
      const std::string& metadata_file_suffix,
      const std::string& sequence_file_suffix,
      const silo::preprocessing::Partitions& partition_descriptor,
      const silo::config::DatabaseConfig& database_config
   );

   virtual silo::DatabaseInfo getDatabaseInfo() const;

   virtual DetailedDatabaseInfo detailedDatabaseInfo() const;

   [[maybe_unused]] void flipBitmaps();

   [[maybe_unused]] void indexAllNucleotideSymbolsN();

   [[maybe_unused]] void naiveIndexAllNucleotideSymbolsN();

   [[maybe_unused]] void saveDatabaseState(
      const std::string& save_directory,
      const silo::preprocessing::Partitions& partition_descriptor
   );

   [[maybe_unused]] [[maybe_unused]] void loadDatabaseState(const std::string& save_directory);

   [[nodiscard]] const PangoLineageAliasLookup& getAliasKey() const;

  private:
   PangoLineageAliasLookup alias_key;
   BitmapSizePerSymbol calculateBitmapSizePerSymbol() const;
   BitmapContainerSize calculateBitmapContainerSizePerGenomeSection(uint32_t section_length) const;
};

std::string buildChunkName(unsigned partition, unsigned chunk);

}  // namespace silo

#endif  // SILO_DATABASE_H
