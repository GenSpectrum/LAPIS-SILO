#include "silo/storage/database_partition.h"

#include "silo/storage/dictionary.h"

namespace silo {

const std::vector<preprocessing::Chunk>& DatabasePartition::getChunks() const {
   return chunks;
}

void DatabasePartition::finalizeBuild(const Dictionary& dict) {
   {  /// Precompute all bitmaps for pango_lineages and -sublineages
      const uint32_t pango_count = dict.getPangoLineageCount();
      std::vector<std::vector<uint32_t>> group_by_lineages(pango_count);
      for (uint32_t sid = 0; sid < sequenceCount; ++sid) {
         const auto lineage = meta_store.sequence_id_to_lineage.at(sid);
         group_by_lineages.at(lineage).push_back(sid);
      }

      meta_store.lineage_bitmaps.resize(pango_count);
      for (uint32_t pango = 0; pango < pango_count; ++pango) {
         meta_store.lineage_bitmaps[pango].addMany(
            group_by_lineages[pango].size(), group_by_lineages[pango].data()
         );
      }

      meta_store.sublineage_bitmaps.resize(pango_count);
      for (uint32_t pango1 = 0; pango1 < pango_count; ++pango1) {
         // Initialize with all lineages that are in pango1
         std::vector<uint32_t> group_by_lineages_sub(group_by_lineages[pango1]);

         // Now add all lineages that I am a prefix of
         for (uint32_t pango2 = 0; pango2 < pango_count; ++pango2) {
            const std::string& str1 = dict.getPangoLineage(pango1);
            const std::string& str2 = dict.getPangoLineage(pango2);
            if (str1.length() >= str2.length()) {
               continue;
            }
            // Check if str1 is a prefix of str2 -> str2 is a sublineage of str1
            if (str2.starts_with(str1)) {
               for (uint32_t const pid : group_by_lineages[pango2]) {
                  group_by_lineages_sub.push_back(pid);
               }
            }
         }
         // Sort, for roaring insert
         std::sort(group_by_lineages_sub.begin(), group_by_lineages_sub.end());
         meta_store.sublineage_bitmaps[pango1].addMany(
            group_by_lineages_sub.size(), group_by_lineages_sub.data()
         );
      }
   }

   {  /// Precompute all bitmaps for countries
      const uint32_t country_count = dict.getCountryCount();
      std::vector<std::vector<uint32_t>> group_by_country(country_count);
      for (uint32_t sid = 0; sid < sequenceCount; ++sid) {
         const auto& country = meta_store.sequence_id_to_country[sid];
         group_by_country[country].push_back(sid);
      }

      meta_store.country_bitmaps.resize(country_count);
      for (uint32_t country = 0; country < country_count; ++country) {
         meta_store.country_bitmaps[country].addMany(
            group_by_country[country].size(), group_by_country[country].data()
         );
      }
   }

   {  /// Precompute all bitmaps for regions
      const uint32_t region_count = dict.getRegionCount();
      std::vector<std::vector<uint32_t>> group_by_region(region_count);
      for (uint32_t sid = 0; sid < sequenceCount; ++sid) {
         const auto& region = meta_store.sequence_id_to_region[sid];
         group_by_region[region].push_back(sid);
      }

      meta_store.region_bitmaps.resize(region_count);
      for (uint32_t region = 0; region < region_count; ++region) {
         meta_store.region_bitmaps[region].addMany(
            group_by_region[region].size(), group_by_region[region].data()
         );
      }
   }
}

}  // namespace silo
