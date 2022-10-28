//
// Created by Alexander Taepper on 26.09.22.
//

#ifndef SILO_META_STORE_H
#define SILO_META_STORE_H

#include "silo.h"

namespace silo {

struct pango_t {
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& ar, const unsigned int /* version */) {
      ar& pango_lineage;
      ar& count;
      ar& partition;
   }
   std::string pango_lineage;
   uint32_t count;
   uint32_t partition;
};

struct partition_t {
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& ar, const unsigned int /* version */) {
      ar& prefix;
      ar& count;
      ar& pids;
   }
   std::string prefix;
   uint32_t count;
   std::vector<uint32_t> pids;
};

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

      ar& pangos;
      ar& partitions;
   }

   std::unordered_map<std::string, std::string> alias_key;

   // Maps the epis to the ID, which is assigned to the pango id (pid)
   // pids are starting at 0 and are dense, so that we can save the respective data in vectors.
   std::unordered_map<uint64_t, uint16_t> epi_to_pid;
   std::unordered_map<uint64_t, uint32_t> epi_to_sidM;

   std::vector<uint64_t> sidM_to_epi;
   std::vector<time_t> sidM_to_date;

   std::vector<std::string> sidM_to_region;
   std::vector<std::string> all_regions;
   std::vector<roaring::Roaring> region_bitmaps;

   std::vector<std::string> sidM_to_country;
   std::vector<std::string> all_countries;
   std::vector<roaring::Roaring> country_bitmaps;

   std::unordered_map<std::string, uint32_t> dictionary;
   std::vector<std::string> dict;

   std::unordered_map<std::string, uint16_t> pango_to_pid;
   std::vector<silo::pango_t> pangos;

   std::vector<silo::partition_t> partitions;
   std::vector<uint32_t> pid_to_partition;

   uint32_t sequence_count = 0;
   uint16_t pid_count = 0;

   MetaStore() {
      std::ifstream alias_key_file("../Data/pango_alias.txt");
      if(!alias_key_file){
         std::cerr << "Expected file Data/pango_alias.txt." << std::endl;
      }
      while (true) {
         std::string alias, val;
         if (!getline(alias_key_file, alias, '\t')) break;
         if (!getline(alias_key_file, val, '\n')) break;
         alias_key[alias] = val;
      }
   }
};
void processMeta(MetaStore& mdb, std::istream& in);

void processMeta_ordered(MetaStore& mdb, std::istream& in);

void pango_info(const MetaStore& mdb, std::ostream& out);

void partition_info(const MetaStore& mdb, std::ostream& out);

unsigned save_meta(const MetaStore& db, const std::string& db_filename);

unsigned load_meta(MetaStore& db, const std::string& db_filename);

std::vector<partition_t> merge_pangos_to_partitions(std::vector<pango_t>& pangos,
                                                    unsigned target_size, unsigned min_size);

static bool resolve_alias(const std::unordered_map<std::string, std::string>& alias_key, std::string& pango_lineage) {
   std::string pango_pref;
   std::stringstream pango_lin_stream(pango_lineage);
   getline(pango_lin_stream, pango_pref, '.');
   if (alias_key.contains(pango_pref)) {
      if (pango_lin_stream.eof()) {
         pango_lineage = alias_key.at(pango_pref);
         return true;
      }
      std::string x((std::istream_iterator<char>(pango_lin_stream)), std::istream_iterator<char>());
      pango_lineage = alias_key.at(pango_pref) + '.' + x;
      return true;
   } else {
      return false;
   }
}

static std::string common_pango_prefix(const std::string& s1, const std::string& s2) {
   std::string prefix;
   // Buffer until it reaches another .
   std::string buffer;
   unsigned min_len = std::min(s1.length(), s2.length());
   for (unsigned i = 0; i < min_len; i++) {
      if (s1[i] != s2[i])
         return prefix;
      else if (s1[i] == '.') {
         prefix += buffer + '.';
         buffer = "";
      } else {
         buffer += s1[i];
      }
   }
   return prefix + buffer;
}

} // namespace silo;

#endif //SILO_META_STORE_H
