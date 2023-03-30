#ifndef SILO_DATABASE_H
#define SILO_DATABASE_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "silo/storage/database_partition.h"
#include "silo/storage/dictionary.h"
#include "silo/storage/metadata_store.h"
#include "silo/storage/pango_lineage_alias.h"
#include "silo/storage/sequence_store.h"

namespace silo {

namespace preprocessing {
struct Partitions;
}

struct PangoLineageCount {
   std::string pango_lineage;
   uint32_t count;
};

struct PangoLineageCounts {
   std::vector<PangoLineageCount> pango_lineage_counts;
};

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
   std::unique_ptr<PangoLineageCounts> pango_descriptor;
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

unsigned fillSequenceStore(SequenceStore& sequence_store, std::istream& input_file);

void savePangoLineageCounts(
   const PangoLineageCounts& pango_lineage_counts,
   std::ostream& output_file
);

PangoLineageCounts loadPangoLineageCounts(std::istream& input_stream);

void savePartitions(const preprocessing::Partitions& partitions, std::ostream& output_file);

preprocessing::Partitions loadPartitions(std::istream& input_file);

std::string buildChunkName(unsigned partition, unsigned chunk);

}  // namespace silo

#endif  // SILO_DATABASE_H
