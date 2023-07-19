#include "silo/preprocessing/partition.h"

#include <algorithm>
#include <cstdlib>
#include <istream>
#include <iterator>
#include <list>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "silo/persistence/exception.h"
#include "silo/preprocessing/pango_lineage_count.h"

namespace silo::preprocessing {

std::string commonPangoPrefix(const std::string& lineage1, const std::string& lineage2) {
   std::string prefix;
   // Buffer until it reaches another .
   std::string buffer;
   const uint32_t min_len = std::min(lineage1.length(), lineage2.length());
   for (uint32_t i = 0; i < min_len; i++) {
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
   const std::vector<PangoLineageCount>& pango_lineage_counts,
   // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
   uint32_t target_size,
   uint32_t min_size
) {
   // Initialize chunks such that every chunk is just a pango_lineage
   std::list<Chunk> chunks;
   for (const auto& count : pango_lineage_counts) {
      std::vector<std::string> pango_lineages;
      pango_lineages.push_back(count.pango_lineage);
      const Chunk tmp = {count.pango_lineage, count.count_of_sequences, 0, pango_lineages};
      chunks.emplace_back(tmp);
   }
   // We want to prioritise merges more closely related chunks.
   // Therefore, we first merge the chunks, with longer matching prefixes.
   // Precalculate the longest a prefix can be (which is the max length of lineages)
   const uint32_t max_len = std::max_element(
                               pango_lineage_counts.begin(),
                               pango_lineage_counts.end(),
                               [](const PangoLineageCount& lhs, const PangoLineageCount& rhs) {
                                  return lhs.pango_lineage.size() < rhs.pango_lineage.size();
                               }
   )->pango_lineage.size();
   for (uint32_t len = max_len; len > 0; len--) {
      for (auto it = chunks.begin(); it != chunks.end() && std::next(it) != chunks.end();) {
         auto&& [pango1, pango2] = std::tie(*it, *std::next(it));
         std::string const common_prefix = commonPangoPrefix(pango1.prefix, pango2.prefix);
         // We only look at possible merges with a common_prefix length of #len
         const bool one_chunk_is_very_small =
            pango1.count_of_sequences < min_size || pango2.count_of_sequences < min_size;
         const bool both_chunks_still_want_to_grow =
            pango1.count_of_sequences < target_size && pango2.count_of_sequences < target_size;
         if (common_prefix.size() == len && (one_chunk_is_very_small || both_chunks_still_want_to_grow)) {
            pango2.prefix = common_prefix;
            pango2.count_of_sequences += pango1.count_of_sequences;
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

   std::vector<Chunk> ret;
   std::copy(std::begin(chunks), std::end(chunks), std::back_inserter(ret));
   return ret;
}

silo::preprocessing::Partition::Partition(std::vector<Chunk>&& chunks_)
    : chunks(chunks_) {
   uint32_t running_total = 0;
   for (Chunk& chunk : chunks) {
      chunk.offset = running_total;
      running_total += chunk.count_of_sequences;
   }
   sequence_count = running_total;
}

const std::vector<Chunk>& silo::preprocessing::Partition::getChunks() const {
   return chunks;
}
uint32_t Partition::getSequenceCount() const {
   return sequence_count;
}

silo::preprocessing::Partitions::Partitions(std::vector<Partition> partitions_)
    : partitions(std::move(partitions_)) {
   for (uint32_t part_id = 0, limit = partitions.size(); part_id < limit; ++part_id) {
      const auto& part = partitions[part_id];
      for (uint32_t chunk_id = 0, limit2 = part.getChunks().size(); chunk_id < limit2; ++chunk_id) {
         const auto& chunk = part.getChunks()[chunk_id];
         partition_chunks.emplace_back(preprocessing::PartitionChunk{
            part_id, chunk_id, chunk.count_of_sequences});
      }
   }

   for (uint32_t i = 0, limit = partitions.size(); i < limit; ++i) {
      const auto& part = partitions[i];
      for (uint32_t j = 0, limit2 = part.getChunks().size(); j < limit2; ++j) {
         const auto& chunk = part.getChunks()[j];
         for (const auto& pango : chunk.pango_lineages) {
            pango_to_chunk[pango] = {i, j, chunk.count_of_sequences};
         }
      }
   }
}

Partitions buildPartitions(
   const silo::preprocessing::PangoLineageCounts& pango_lineage_counts,
   silo::preprocessing::Architecture arch
) {
   std::vector<Partition> partitions;
   uint32_t total_count_of_sequences = 0;
   for (const auto& pango_lineage_count : pango_lineage_counts.pango_lineage_counts) {
      total_count_of_sequences += pango_lineage_count.count_of_sequences;
   }
   constexpr int TARGET_SIZE_REDUCTION = 100;
   constexpr int MIN_SIZE_REDUCTION = 200;

   if (arch == Architecture::MAX_PARTITIONS) {
      for (auto& chunk : mergePangosToChunks(
              pango_lineage_counts.pango_lineage_counts,
              total_count_of_sequences / 100,
              total_count_of_sequences / 200
           )) {
         partitions.emplace_back(std::vector<Chunk>{{chunk}});
      }
   } else if (arch == Architecture::SINGLE_PARTITION) {
      // Merge pango_lineages, such that chunks are not get very small
      std::vector<Chunk> chunks = mergePangosToChunks(
         pango_lineage_counts.pango_lineage_counts,
         total_count_of_sequences / TARGET_SIZE_REDUCTION,
         total_count_of_sequences / MIN_SIZE_REDUCTION
      );

      partitions.emplace_back(std::move(chunks));
   } else if (arch == Architecture::SINGLE_SINGLE) {
      // Merge pango_lineages, such that all lineages are in one chunk
      Chunk chunk;
      for (const auto& pango : pango_lineage_counts.pango_lineage_counts) {
         chunk.pango_lineages.push_back(pango.pango_lineage);
      }
      partitions.emplace_back(std::vector<Chunk>{{chunk}});
   }
   return Partitions{partitions};
}

const std::vector<Partition>& Partitions::getPartitions() const {
   return partitions;
}

const std::vector<PartitionChunk>& Partitions::getPartitionChunks() const {
   return partition_chunks;
}

const std::unordered_map<std::string, silo::preprocessing::PartitionChunk>& Partitions::
   getPangoToChunk() const {
   return pango_to_chunk;
}

bool PartitionChunk::operator==(const PartitionChunk& other) const {
   return partition == other.partition && chunk == other.chunk && size == other.size;
}

}  // namespace silo::preprocessing

std::size_t std::hash<silo::preprocessing::PartitionChunk>::operator()(
   const silo::preprocessing::PartitionChunk& partition_chunk
) const {
   return hash<uint32_t>()(partition_chunk.partition) +
          (hash<uint32_t>()(partition_chunk.chunk) << 3) +
          (hash<uint32_t>()(partition_chunk.chunk) >> 2);
}

namespace silo::preprocessing {

void Partitions::save(std::ostream& output_file) const {
   for (const auto& partition : partitions) {
      output_file << "P\t" << partition.getChunks().size() << '\t' << partition.getSequenceCount()
                  << '\n';
      for (const auto& chunk : partition.getChunks()) {
         output_file << "C\t" << chunk.prefix << '\t' << chunk.pango_lineages.size() << '\t'
                     << chunk.count_of_sequences << '\t' << chunk.offset << '\n';
         for (const auto& pango_lineage : chunk.pango_lineages) {
            output_file << "L\t" << pango_lineage << '\n';
         }
      }
   }
}
}  // namespace silo::preprocessing
