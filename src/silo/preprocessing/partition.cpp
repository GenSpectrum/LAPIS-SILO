#include "silo/preprocessing/partition.h"

#include "silo/database.h"

namespace silo::preprocessing {

static std::string commonPangoPrefix(const std::string& lineage1, const std::string& lineage2) {
   std::string prefix;
   // Buffer until it reaches another .
   std::string buffer;
   unsigned const min_len = std::min(lineage1.length(), lineage2.length());
   for (unsigned i = 0; i < min_len; i++) {
      if (lineage1[i] != lineage2[i]) {
         return prefix;
      }
      if (lineage1[i] == '.') {
         prefix += buffer + '.';
         buffer = "";
      } else {
         buffer += lineage1[i];
      }
   }
   return prefix + buffer;
}

/// Takes pango_lineages as initial chunk and merges them, trying to merge more closely related ones
/// first Will merge 2 chunks if on is smaller than min_size or both are smaller than target_size
/// Updates pango_lineages to contain the chunk each pango_lineage is contained in and returns
/// vector of chunks
std::vector<silo::preprocessing::Chunk> mergePangosToChunks(
   std::vector<silo::PangoLineageCount>& pango_lineage_counts,
   unsigned target_size,
   unsigned min_size
) {
   // Initialize chunks such that every chunk is just a pango_lineage
   std::list<silo::preprocessing::Chunk> chunks;
   uint32_t running_total = 0;
   for (auto& count : pango_lineage_counts) {
      std::vector<std::string> pango_lineages;
      pango_lineages.push_back(count.pango_lineage);
      silo::preprocessing::Chunk const tmp = {
         count.pango_lineage, count.count, running_total, pango_lineages};
      running_total += count.count;
      chunks.emplace_back(tmp);
   }
   // We want to prioritise merges more closely related chunks.
   // Therefore, we first merge the chunks, with longer matching prefixes.
   // Precalculate the longest a prefix can be (which is the max length of lineages)
   uint32_t const max_len =
      std::max_element(
         pango_lineage_counts.begin(),
         pango_lineage_counts.end(),
         [](const silo::PangoLineageCount& lhs, const silo::PangoLineageCount& rhs) {
            return lhs.pango_lineage.size() < rhs.pango_lineage.size();
         }
      )->pango_lineage.size();
   for (uint32_t len = max_len; len > 0; len--) {
      for (auto it = chunks.begin(); it != chunks.end() && std::next(it) != chunks.end();) {
         auto&& [pango1, pango2] = std::tie(*it, *std::next(it));
         std::string const common_prefix = commonPangoPrefix(pango1.prefix, pango2.prefix);
         // We only look at possible merges with a common_prefix length of #len
         bool const one_chunk_is_very_small = pango1.count < min_size || pango2.count < min_size;
         bool const both_chunks_still_want_to_grow =
            pango1.count < target_size && pango2.count < target_size;
         if (common_prefix.size() == len && (one_chunk_is_very_small || both_chunks_still_want_to_grow)) {
            pango2.prefix = common_prefix;
            pango2.count += pango1.count;
            pango2.pango_lineages.insert(
               pango2.pango_lineages.end(),
               pango1.pango_lineages.begin(),
               pango1.pango_lineages.end()
            );

            // We merged pango1 into pango2 -> Now delete pango1
            // Do not need to increment, because erase will make it automatically point to next
            // element
            it = chunks.erase(it);
         } else {
            ++it;
         }
      }
   }

   std::vector<silo::preprocessing::Chunk> ret;
   std::copy(std::begin(chunks), std::end(chunks), std::back_inserter(ret));
   return ret;
}

Partitions buildPartitions(silo::PangoLineageCounts pango_lineage_counts, Architecture arch) {
   uint32_t total_count = 0;
   for (auto& pango_lineage_count : pango_lineage_counts.pango_lineage_counts) {
      total_count += pango_lineage_count.count;
   }

   Partitions descriptor;
   constexpr int TARGET_SIZE_REDUCTION = 100;
   constexpr int MIN_SIZE_REDUCTION = 200;

   switch (arch) {
      case Architecture::MAX_PARTITIONS:
         for (auto& chunk : mergePangosToChunks(
                 pango_lineage_counts.pango_lineage_counts, total_count / 100, total_count / 200
              )) {
            descriptor.partitions.push_back(Partition{});
            descriptor.partitions.back().name = "full";
            descriptor.partitions.back().chunks.push_back(chunk);
            descriptor.partitions.back().count = chunk.count;
         }
         return descriptor;
      case Architecture::SINGLE_PARTITION:
         descriptor.partitions.push_back(Partition{});

         descriptor.partitions[0].name = "full";

         // Merge pango_lineages, such that chunks are not get very small
         descriptor.partitions[0].chunks = mergePangosToChunks(
            pango_lineage_counts.pango_lineage_counts,
            total_count / TARGET_SIZE_REDUCTION,
            total_count / MIN_SIZE_REDUCTION
         );

         descriptor.partitions[0].count = total_count;
         return descriptor;
      case Architecture::SINGLE_SINGLE:

         descriptor.partitions.push_back(Partition{});
         descriptor.partitions[0].name = "full_full";

         // Merge pango_lineages, such that chunks are not get very small
         descriptor.partitions[0].chunks.push_back(Chunk{
            "", total_count, 0, std::vector<std::string>()});
         for (auto& pango : pango_lineage_counts.pango_lineage_counts) {
            descriptor.partitions[0].chunks.back().pango_lineages.push_back(pango.pango_lineage);
         }

         descriptor.partitions[0].count = total_count;
         return descriptor;
      case HYBRID:
         break;
   }
   throw std::runtime_error("Arch not yet implemented.");
}

}  // namespace silo::preprocessing
