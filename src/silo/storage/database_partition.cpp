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
}

}  // namespace silo
