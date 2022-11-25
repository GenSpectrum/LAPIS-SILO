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
      ar& sid_to_epi;
      ar& sid_to_date;
      ar& sid_to_lineage;
      ar& sid_to_region;
      ar& sid_to_country;
   }

   std::vector<uint64_t> sid_to_epi;
   std::vector<time_t> sid_to_date;

   // TODO only ints -> Dictionary:
   std::vector<uint32_t> sid_to_lineage;
   std::vector<roaring::Roaring> lineage_bitmaps;

   std::vector<uint32_t> sid_to_region;
   std::vector<roaring::Roaring> region_bitmaps;

   std::vector<uint32_t> sid_to_country;
   std::vector<roaring::Roaring> country_bitmaps;

   std::vector<std::vector<uint32_t>> columns;
};

void inputSequenceMeta(MetaStore& mdb, uint64_t epi, const std::string& pango_lineage, const std::string& date,
                       const std::string& region, const std::string& country, const std::string& division);

void chunk_info(const MetaStore& mdb, std::ostream& out);

unsigned save_meta(const MetaStore& db, const std::string& db_filename);

unsigned load_meta(MetaStore& db, const std::string& db_filename);

} // namespace silo;

#endif //SILO_META_STORE_H
