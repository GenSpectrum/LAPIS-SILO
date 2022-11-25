//
// Created by Alexander Taepper on 01.09.22.
//

#include "silo/meta_store.h"

using namespace silo;

void silo::inputSequenceMeta(MetaStore& mdb, uint64_t epi, const std::string& pango_lineage, const std::string& date,
                             const std::string& region, const std::string& country, const std::string& /*TODO division*/) {
   mdb.sid_to_epi.push_back(epi);
   mdb.sid_to_lineage.push_back(pango_lineage);

   struct std::tm tm {};
   std::istringstream ss(date);
   ss >> std::get_time(&tm, "%Y-%m-%d");
   std::time_t time = mktime(&tm);

   mdb.sid_to_date.push_back(time);
   mdb.sid_to_country.push_back(country);
   mdb.sid_to_region.push_back(region);
}
