//
// Created by Alexander Taepper on 16.11.22.
//

#ifndef SILO_DATABASE_H
#define SILO_DATABASE_H

#include <silo/meta_store.h>
#include <silo/query_engine.h>
#include <silo/sequence_store.h>
#include <silo/silo.h>

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

struct DatabasePartition {
   MetaStore meta_store;
   SequenceStore seq_store;
};

class Database {
   private:
   std::vector<DatabasePartition> partitions;
   std::unordered_map<std::string, std::string> alias_key;

   public:
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
};

void processSeq(SequenceStore& seq_store, std::istream& in);

void processMeta(MetaStore& meta_store, std::istream& in, const std::unordered_map<std::string, std::string> alias_key);

} // namespace silo

#endif //SILO_DATABASE_H
