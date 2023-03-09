//
// Created by Alexander Taepper on 01.09.22.
//

#include <silo/storage/meta_store.h>

void silo::inputSequenceMeta(
   MetaStore& mdb,
   uint64_t epi,
   time_t date,
   uint32_t pango_lineage,
   uint32_t region,
   uint32_t country,
   const std::vector<uint64_t>& vals
) {
   mdb.sid_to_epi.push_back(epi);
   mdb.sid_to_lineage.push_back(pango_lineage);

   mdb.sid_to_date.push_back(date);
   mdb.sid_to_country.push_back(country);
   mdb.sid_to_region.push_back(region);
   for (unsigned i = 0; i < mdb.cols.size(); ++i) {
      mdb.cols[i].push_back(vals[i]);
   }
}
