//
// Created by Alexander Taepper on 26.09.22.
//

#ifndef SILO_META_STORE_H
#define SILO_META_STORE_H

#include "silo/common/silo_symbols.h"
#include "silo/roaring/roaring.hh"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <silo/roaring/roaring_serialize.h>

namespace silo {

struct MetaStore {
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& ar, const unsigned int /* version */) {
      ar& sid_to_epi;

      ar& sid_to_date;

      ar& sid_to_lineage;
      ar& lineage_bitmaps;
      ar& sublineage_bitmaps;

      ar& sid_to_region;
      ar& region_bitmaps;

      ar& sid_to_country;
      ar& country_bitmaps;

      ar& cols;
   }

   std::vector<uint64_t> sid_to_epi;
   std::vector<time_t> sid_to_date;

   // TODO only ints -> Dictionary:
   std::vector<uint32_t> sid_to_lineage;
   std::vector<roaring::Roaring> lineage_bitmaps;
   std::vector<roaring::Roaring> sublineage_bitmaps;

   std::vector<uint32_t> sid_to_region;
   std::vector<roaring::Roaring> region_bitmaps;

   std::vector<uint32_t> sid_to_country;
   std::vector<roaring::Roaring> country_bitmaps;

   std::vector<std::vector<uint64_t>> cols;
};

void inputSequenceMeta(MetaStore& mdb, uint64_t epi, time_t date, uint32_t pango_lineage,
                       uint32_t region, uint32_t country, const std::vector<uint64_t>& vals);

void chunk_info(const MetaStore& mdb, std::ostream& out);

unsigned save_meta(const MetaStore& db, const std::string& db_filename);

unsigned load_meta(MetaStore& db, const std::string& db_filename);

} // namespace silo;

#endif //SILO_META_STORE_H
