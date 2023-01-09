//
// Created by Alexander Taepper on 16.11.22.
//

#ifndef SILO_DATABASE_H
#define SILO_DATABASE_H

#include <silo/db_components/meta_store.h>
#include <silo/db_components/sequence_store.h>
#include <silo/silo.h>

#include <utility>
#include <silo/db_components/Dictionary.h>

namespace silo {

struct partition_t {
   std::string name;
   uint32_t count;
   std::vector<chunk_t> chunks;
};

struct partitioning_descriptor_t {
   std::vector<partition_t> partitions;
};

struct pango_descriptor_t {
   std::vector<pango_t> pangos;
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
   // Sorted Lineage ids that are contained in this partition (for expression simplification)
   std::vector<uint32_t> sorted_lineages;

   MetaStore meta_store;
   SequenceStore seq_store;
   unsigned sequenceCount;

   const std::vector<silo::chunk_t>& get_chunks() const {
      return chunks;
   }

   void finalize(const Dictionary& dict);
};

class Database {
   public:
   const std::string wd; // working directory
   std::vector<std::string> global_reference;
   std::vector<DatabasePartition> partitions;
   std::unique_ptr<pango_descriptor_t> pango_def;
   std::unique_ptr<partitioning_descriptor_t> part_def;
   std::unique_ptr<Dictionary> dict;

   const std::unordered_map<std::string, std::string> get_alias_key() {
      return alias_key;
   }

   Database(const std::string& wd) : wd(wd) {
      std::ifstream reference_file(wd + "reference_genome.txt");
      if (!reference_file) {
         std::cerr << "Expected file " << wd << "reference_genome.txt." << std::endl;
         return;
      }
      while (true) {
         std::string tmp;
         if (!getline(reference_file, tmp, '\n')) break;
         global_reference.push_back(tmp);
      }
      if (global_reference.empty()) {
         std::cerr << "No genome in " << wd << "reference_genome.txt." << std::endl;
         return;
      }

      std::ifstream alias_key_file(wd + "pango_alias.txt");
      if (!alias_key_file) {
         std::cerr << "Expected file " << wd << "pango_alias.txt." << std::endl;
         return;
      }
      while (true) {
         std::string alias, val;
         if (!getline(alias_key_file, alias, '\t')) break;
         if (!getline(alias_key_file, val, '\n')) break;
         alias_key[alias] = val;
      }
   }

   void build(const std::string& part_prefix, const std::string& meta_suffix, const std::string& seq_suffix);
   // void analyse();
   int db_info(std::ostream& io);
   int db_info_detailed(std::ostream& io);
   void finalize();

   void save(const std::string& save_dir);

   void load(const std::string& save_dir);
   std::unordered_map<std::string, std::string> alias_key;
};

unsigned processSeq(SequenceStore& seq_store, std::istream& in);

unsigned processMeta(MetaStore& meta_store, std::istream& in, const std::unordered_map<std::string, std::string>& alias_key, const Dictionary& dict);

void save_pango_defs(const pango_descriptor_t& pd, std::ostream& out);

pango_descriptor_t load_pango_defs(std::istream& in);

void save_partitioning_descriptor(const partitioning_descriptor_t& pd, std::ostream& out);

partitioning_descriptor_t load_partitioning_descriptor(std::istream& in);

} // namespace silo

#endif //SILO_DATABASE_H
