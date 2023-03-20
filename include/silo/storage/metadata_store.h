#ifndef SILO_META_STORE_H
#define SILO_META_STORE_H

#include <roaring/roaring.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <ctime>
#include <vector>

#include "silo/common/silo_symbols.h"
#include "silo/roaring/roaring_serialize.h"

namespace silo {

struct MetadataStore {
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive& sequence_id_to_epi;

      archive& sequence_id_to_date;

      archive& sequence_id_to_lineage;
      archive& lineage_bitmaps;
      archive& sublineage_bitmaps;

      archive& sequence_id_to_region;
      archive& region_bitmaps;

      archive& sequence_id_to_country;
      archive& country_bitmaps;

      archive& columns;
   }

   std::vector<uint64_t> sequence_id_to_epi;
   std::vector<time_t> sequence_id_to_date;

   // TODO(taepper) only ints -> Dictionary:
   std::vector<uint32_t> sequence_id_to_lineage;
   std::vector<roaring::Roaring> lineage_bitmaps;
   std::vector<roaring::Roaring> sublineage_bitmaps;

   std::vector<uint32_t> sequence_id_to_region;
   std::vector<roaring::Roaring> region_bitmaps;

   std::vector<uint32_t> sequence_id_to_country;
   std::vector<roaring::Roaring> country_bitmaps;

   std::vector<std::vector<uint64_t>> columns;
};

void inputSequenceMeta(
   MetadataStore& metadata_store,
   uint64_t epi_isl_number,
   time_t date,
   uint32_t pango_lineage,
   uint32_t region,
   uint32_t country,
   const std::vector<uint64_t>& values
);

}  // namespace silo

#endif  // SILO_META_STORE_H
