//
// Created by Alexander Taepper on 26.09.22.
//

#ifndef SILO_META_STORE_H
#define SILO_META_STORE_H

#include "silo.h"

namespace silo {

   struct MetaStore {
      friend class boost::serialization::access;

      template<class Archive>
      void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
         ar & pid_count;
         ar & epi_count;

         // unordered_map
         ar & epi_to_pid;
         ar & epi_to_sidM;

         // vector
         ar & sidM_to_epi;
         ar & sidM_to_date;
         ar & sidM_to_region;
         ar & sidM_to_country;

         // unordered_map
         ar & pango_to_pid;

         // vector
         ar & pid_to_pango;
         ar & pid_to_metacount;
      }

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

      uint32_t epi_count = 0;
      uint16_t pid_count = 0;
   };

// Meta-Data is input
   void analyseMeta(std::istream &in);

   void processMeta(MetaStore &mdb, std::istream &in);

   void processMeta_ordered(MetaStore &mdb, std::istream &in);

   void meta_info(const MetaStore &mdb, std::ostream &out);

   unsigned save_meta(const MetaStore &db, const std::string &db_filename);

   unsigned load_meta(MetaStore &db, const std::string &db_filename);

} // namespace silo;

#endif //SILO_META_STORE_H