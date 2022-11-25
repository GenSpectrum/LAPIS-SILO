//
// Created by Alexander Taepper on 16.11.22.
//

#ifndef SILO_DATABASE_H
#define SILO_DATABASE_H

#include <silo/meta_store.h>
#include <silo/sequence_store.h>
#include <silo/silo.h>

#include <utility>

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

class Dictionary {
   private:
   std::unordered_map<std::string_view, uint32_t> pango_dict;
   std::unordered_map<std::string_view, uint32_t> country_dict;
   std::unordered_map<std::string_view, uint32_t> region_dict;
   std::unordered_map<std::string_view, uint32_t> general_dict;

   std::vector<std::string> pango_lookup;
   std::vector<std::string> country_lookup;
   std::vector<std::string> region_lookup;
   std::vector<std::string> general_lookup;

   uint32_t next_pango_id;
   uint32_t next_country_id;
   uint32_t next_region_id;
   uint32_t next_general_id;

   public:
   uint32_t get_pangoid(const std::string& str);

   const std::string& get_pango(uint32_t id);

   uint32_t get_countryid(const std::string& str);

   const std::string& get_country(uint32_t id);

   uint32_t get_regionid(const std::string& str);

   const std::string& get_region(uint32_t id);

   uint32_t get_id(const std::string& str);

   const std::string& get_str(uint32_t id);

   void save_dict(std::ostream& dict_file);
};

class DatabasePartition {
   friend class Database;
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
      ar& meta_store;
      ar& seq_store;
   }

   std::vector<silo::chunk_t> chunks;

   public:
   MetaStore meta_store;
   SequenceStore seq_store;

   const std::vector<silo::chunk_t>& get_chunks() const {
      return chunks;
   }

   unsigned sequenceCount;
   void finalize();
};

class Database {
   private:
   std::unordered_map<std::string, std::string> alias_key;

   std::unordered_map<std::string, uint32_t> dict_lookup;
   std::vector<std::string> dict;

   public:
   std::vector<DatabasePartition> partitions;
   std::unique_ptr<pango_descriptor_t> pango_def;
   std::unique_ptr<partitioning_descriptor_t> part_def;

   const std::unordered_map<std::string, std::string> get_alias_key() {
      return alias_key;
   }

   Database() {
      std::ifstream alias_key_file("../Data/pango_alias.txt");
      if (!alias_key_file) {
         std::cerr << "Expected file Data/pango_alias.txt." << std::endl;
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
};

unsigned processSeq(SequenceStore& seq_store, std::istream& in);

unsigned processMeta(MetaStore& meta_store, std::istream& in, const std::unordered_map<std::string, std::string>& alias_key);

void save_pango_defs(const pango_descriptor_t& pd, std::ostream& out);

pango_descriptor_t load_pango_defs(std::istream& in);

void save_partitioning_descriptor(const partitioning_descriptor_t& pd, std::ostream& out);

partitioning_descriptor_t load_partitioning_descriptor(std::istream& in);

} // namespace silo

#endif //SILO_DATABASE_H
