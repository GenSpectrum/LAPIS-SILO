#ifndef SILO_DATABASE_H
#define SILO_DATABASE_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "silo/preprocessing/pango_lineage_count.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/dictionary.h"
#include "silo/storage/pango_lineage_alias.h"

namespace silo {

namespace preprocessing {

struct Partitions;

}  // namespace preprocessing

struct DatabaseInfo;
struct DetailedDatabaseInfo;
struct BitmapSizePerSymbol;
struct BitmapContainerSize;

struct PreprocessingConfig;

class Database {
  public:
   const std::string working_directory;
   std::vector<std::string> global_reference;
   std::vector<DatabasePartition> partitions;
   std::unique_ptr<preprocessing::PangoLineageCounts> pango_descriptor;
   std::unique_ptr<preprocessing::Partitions> partition_descriptor;
   std::unique_ptr<Dictionary> dict;

   Database();

   explicit Database(const std::string& directory);

   void preprocessing(const PreprocessingConfig& config);

   void build(
      const std::string& partition_name_prefix,
      const std::string& metadata_file_suffix,
      const std::string& sequence_file_suffix
   );

   virtual silo::DatabaseInfo getDatabaseInfo() const;

   virtual DetailedDatabaseInfo detailedDatabaseInfo() const;
   void finalizeBuild();

   [[maybe_unused]] void flipBitmaps();

   [[maybe_unused]] void indexAllNucleotideSymbolsN();

   [[maybe_unused]] void naiveIndexAllNucleotideSymbolsN();

   [[maybe_unused]] void saveDatabaseState(const std::string& save_directory);

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
