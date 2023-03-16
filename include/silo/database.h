#ifndef SILO_DATABASE_H
#define SILO_DATABASE_H

#include <silo/common/silo_symbols.h>
#include <silo/storage/Dictionary.h>
#include <silo/storage/meta_store.h>
#include <silo/storage/sequence_store.h>

#include <utility>

namespace silo {

struct Chunk {
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, [[maybe_unused]] const unsigned int version) {
      archive& prefix;
      archive& count;
      archive& offset;
      archive& pango_lineages;
   }
   std::string prefix;
   uint32_t count;
   uint32_t offset;
   std::vector<std::string> pango_lineages;
};

struct Partition {
   std::string name;
   uint32_t count;
   std::vector<Chunk> chunks;
};

struct Partitions {
   std::vector<Partition> partitions;
};

struct PangoLineageCount {
   std::string pango_lineage;
   uint32_t count;
};

struct PangoLineageCounts {
   std::vector<PangoLineageCount> pango_lineage_counts;
};

struct DatabaseInfo {
   uint32_t sequenceCount;
   uint64_t totalSize;
   size_t nBitmapsSize;
};

class DatabasePartition {
   friend class Database;
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const unsigned int version) {
      archive& meta_store;
      archive& seq_store;
      archive& sequenceCount;
      archive& chunks;
   }

   std::vector<silo::Chunk> chunks;

  public:
   MetadataStore meta_store;
   SequenceStore seq_store;
   unsigned sequenceCount;

   [[nodiscard]] const std::vector<silo::Chunk>& getChunks() const;

   void finalizeBuild(const Dictionary& dict);
};

struct PreprocessingConfig;

class Database {
  public:
   const std::string working_directory;
   std::vector<std::string> global_reference;
   std::vector<DatabasePartition> partitions;
   std::unique_ptr<PangoLineageCounts> pango_descriptor;
   std::unique_ptr<Partitions> partition_descriptor;
   std::unique_ptr<Dictionary> dict;

   Database();

   explicit Database(const std::string& directory);

   void preprocessing(const PreprocessingConfig& config);

   void build(
      const std::string& partition_name_prefix,
      const std::string& metadata_file_suffix,
      const std::string& sequence_file_suffix,
      std::ostream& out
   );

   virtual silo::DatabaseInfo getDatabaseInfo();

   int detailedDatabaseInfo(std::ostream& output_file);
   [[maybe_unused]] void printFlippedGenomePositions(std::ostream& output_file);
   void finalizeBuild();

   [[maybe_unused]] void flipBitmaps();

   [[maybe_unused]] void indexAllNucleotideSymbolsN();

   [[maybe_unused]] void naiveIndexAllNucleotideSymbolsN();

   [[maybe_unused]] void saveDatabaseState(const std::string& save_directory);

   [[maybe_unused]] [[maybe_unused]] void loadDatabaseState(const std::string& save_directory);

   const std::unordered_map<std::string, std::string>& getAliasKey() const;

  private:
   std::unordered_map<std::string, std::string> alias_key;
};

unsigned fillSequenceStore(SequenceStore& seq_store, std::istream& input_file);

unsigned fillMetadataStore(
   MetadataStore& meta_store,
   std::istream& input_file,
   const std::unordered_map<std::string, std::string>& alias_key,
   const Dictionary& dict
);

void savePangoLineageCounts(
   const PangoLineageCounts& pango_lineage_counts,
   std::ostream& output_file
);

PangoLineageCounts loadPangoLineageCounts(std::istream& input_stream);

void savePartitions(const Partitions& partitions, std::ostream& output_file);

Partitions loadPartitions(std::istream& input_file);

std::string formatNumber(uint64_t number);

std::string resolvePangoLineageAlias(
   const std::unordered_map<std::string, std::string>& alias_key,
   const std::string& pango_lineage
);

std::string buildChunkName(unsigned partition, unsigned chunk);

}  // namespace silo

#endif  // SILO_DATABASE_H
