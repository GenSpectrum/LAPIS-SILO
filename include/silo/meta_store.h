//
// Created by Alexander Taepper on 26.09.22.
//

#ifndef SILO_META_STORE_H
#define SILO_META_STORE_H

#include "silo.h"

namespace silo {

struct MetaStore {
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& ar, const unsigned int /* version */) {
      ar& sequence_count;
      ar& pid_count;

      ar& alias_key;

      ar& epi_to_pid;
      ar& epi_to_sidM;

      ar& sidM_to_epi;
      ar& sidM_to_date;
      ar& sidM_to_region;
      ar& sidM_to_country;

      ar& pango_to_pid;

      ar& pid_to_pango;
      ar& pid_to_metacount;
   }

   std::unordered_map<std::string, std::string> alias_key;

   // Maps the epis to the ID, which is assigned to the pango lineage (pid)
   // pids are starting at 0 and are dense, so that we can save the respective data in vectors.
   std::unordered_map<uint64_t, uint16_t> epi_to_pid;
   std::unordered_map<uint64_t, uint32_t> epi_to_sidM;

   std::vector<uint64_t> sidM_to_epi;
   std::vector<time_t> sidM_to_date;
   std::vector<std::string> sidM_to_region;
   std::vector<std::string> sidM_to_country;

   std::unordered_map<std::string, uint16_t> pango_to_pid;

   std::vector<std::string> pid_to_pango;
   // counts the occurence of each pid in the metadata
   std::vector<uint32_t> pid_to_metacount;

   uint32_t sequence_count = 0;
   uint16_t pid_count = 0;

   MetaStore() {
      std::ifstream alias_key_file("../Data/pango_alias.txt");
      while (true) {
         std::string alias, val;
         if (!getline(alias_key_file, alias, '\t')) break;
         if (!getline(alias_key_file, val, '\n')) break;
         alias_key[alias] = val;
      }
   }
};

// Meta-Data is input
void analyseMeta(std::istream& in);

void processMeta(MetaStore& mdb, std::istream& in);

void processMeta_ordered(MetaStore& mdb, std::istream& in);

void meta_info(const MetaStore& mdb, std::ostream& out);

unsigned save_meta(const MetaStore& db, const std::string& db_filename);

unsigned load_meta(MetaStore& db, const std::string& db_filename);

} // namespace silo;

#endif //SILO_META_STORE_H
