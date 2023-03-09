#ifndef SILO_DATABASE_H
#define SILO_DATABASE_H

#include <silo/common/silo_symbols.h>
#include <silo/storage/Dictionary.h>
#include <silo/storage/meta_store.h>
#include <silo/storage/sequence_store.h>

#include <utility>

namespace silo {

struct chunk_t {
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& ar, const unsigned int /* version */) {
      ar& prefix;
      ar& count;
      ar& offset;
      ar& pangos;
   }
   std::string prefix;
   uint32_t count;
   uint32_t offset;
   std::vector<std::string> pangos;
};

struct partition_t {
   std::string name;
   uint32_t count;
   std::vector<chunk_t> chunks;
};

struct partitioning_descriptor_t {
   std::vector<partition_t> partitions;
};

struct pango_t {
   std::string pango_lineage;
   uint32_t count;
};

struct pango_descriptor_t {
   std::vector<pango_t> pangos;
};

struct db_info_t {
   uint32_t sequenceCount;
   uint64_t totalSize;
   size_t nBitmapsSize;
};

class DatabasePartition {
   friend class Database;
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
      ar& meta_store;
      ar& seq_store;
      ar& sequenceCount;
      ar& chunks;
   }

   std::vector<silo::chunk_t> chunks;

  public:
   MetaStore meta_store;
   SequenceStore seq_store;
   unsigned sequenceCount;

   const std::vector<silo::chunk_t>& get_chunks() const { return chunks; }

   void finalizeBuild(const Dictionary& dict);
};

struct PreprocessingConfig;

class Database {
  public:
   const std::string wd;  // working directory
   std::vector<std::string> global_reference;
   std::vector<DatabasePartition> partitions;
   std::unique_ptr<pango_descriptor_t> pango_descriptor;
   std::unique_ptr<partitioning_descriptor_t> partition_descriptor;
   std::unique_ptr<Dictionary> dict;

   Database(){};

   Database(const std::string& wd);

   void preprocessing(const PreprocessingConfig& config);

   void build(
      const std::string& part_prefix,
      const std::string& meta_suffix,
      const std::string& seq_suffix,
      std::ostream& out
   );

   virtual silo::db_info_t get_db_info();

   int db_info_detailed(std::ostream& io);
   void print_flipped(std::ostream& io);
   void finalizeBuild();

   void flipBitmaps();

   void indexAllN();

   void indexAllN_naive();

   void save(const std::string& save_dir);

   void load(const std::string& save_dir);

   const std::unordered_map<std::string, std::string>& getAliasKey() const;

  private:
   std::unordered_map<std::string, std::string> alias_key;
};

unsigned processSeq(SequenceStore& seq_store, std::istream& in);

unsigned processMeta(
   MetaStore& meta_store,
   std::istream& in,
   const std::unordered_map<std::string, std::string>& alias_key,
   const Dictionary& dict
);

void save_pango_defs(const pango_descriptor_t& pd, std::ostream& out);

pango_descriptor_t load_pango_defs(std::istream& in);

void save_partitioning_descriptor(const partitioning_descriptor_t& pd, std::ostream& out);

partitioning_descriptor_t load_partitioning_descriptor(std::istream& in);

}  // namespace silo

#endif  // SILO_DATABASE_H
