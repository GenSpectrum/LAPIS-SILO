#ifndef SILO_DATABASE_H
#define SILO_DATABASE_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "silo/config/database_config.h"
#include "silo/preprocessing/pango_lineage_count.h"
#include "silo/storage/pango_lineage_alias.h"
#include "silo/storage/reference_genome.h"

namespace silo {
struct DatabasePartition;

namespace storage::column {
struct DateColumn;
struct IndexedStringColumn;
struct StringColumn;
struct PangoLineageColumn;
struct IntColumn;
}  // namespace storage::column

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
   std::unique_ptr<ReferenceGenome> reference_genome;
   std::vector<DatabasePartition> partitions;

   std::unordered_map<std::string, storage::column::StringColumn> string_columns;
   std::unordered_map<std::string, storage::column::IndexedStringColumn> indexed_string_columns;
   std::unordered_map<std::string, storage::column::IntColumn> int_columns;
   std::unordered_map<std::string, storage::column::DateColumn> date_columns;
   std::unordered_map<std::string, storage::column::PangoLineageColumn> pango_lineage_columns;

   Database();

   void preprocessing(
      const preprocessing::PreprocessingConfig& preprocessing_config,
      const config::DatabaseConfig& database_config_
   );

   void build(
      const std::string& partition_name_prefix,
      const std::string& metadata_file_suffix,
      const std::string& sequence_file_suffix,
      const silo::preprocessing::Partitions& partition_descriptor
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

   const silo::storage::column::DateColumn& getDateColumn(std::string name) const;

   const silo::storage::column::IndexedStringColumn& getIndexedStringColumn(std::string name) const;

   const silo::storage::column::StringColumn& getStringColumn(std::string name) const;

   const silo::storage::column::PangoLineageColumn& getPangoLineageColumn(std::string name) const;

   const silo::storage::column::IntColumn& getIntColumn(std::string name) const;

  private:
   PangoLineageAliasLookup alias_key;

   void initializeColumns();

   BitmapSizePerSymbol calculateBitmapSizePerSymbol() const;

   BitmapContainerSize calculateBitmapContainerSizePerGenomeSection(uint32_t section_length) const;
};

std::string buildChunkName(unsigned partition, unsigned chunk);

}  // namespace silo

#endif  // SILO_DATABASE_H
