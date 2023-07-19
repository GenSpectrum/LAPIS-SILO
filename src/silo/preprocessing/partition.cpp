#include "silo/preprocessing/partition.h"

#include <algorithm>
#include <cstdlib>
#include <istream>
#include <iterator>
#include <list>
#include <stdexcept>
#include <tuple>
#include <utility>

#include <nlohmann/json.hpp>

#include "silo/persistence/exception.h"
#include "silo/preprocessing/pango_lineage_count.h"

namespace silo::preprocessing {

std::string commonPangoPrefix(std::string_view lineage1, std::string_view lineage2) {
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
   for (const auto& [lineage, count] : pango_lineage_counts) {
      chunks.emplace_back(lineage, count);
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
         const std::string common_prefix =
            commonPangoPrefix(pango1.getPrefix(), pango2.getPrefix());
         // We only look at possible merges with a common_prefix length of #len
         const bool one_chunk_is_very_small =
            pango1.getCountOfSequences() < min_size || pango2.getCountOfSequences() < min_size;
         const bool both_chunks_still_want_to_grow = pango1.getCountOfSequences() < target_size &&
                                                     pango2.getCountOfSequences() < target_size;
         if (common_prefix.size() == len && (one_chunk_is_very_small || both_chunks_still_want_to_grow)) {
            pango2.addChunk(std::move(pango1));

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
      running_total += chunk.getCountOfSequences();
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
            part_id, chunk_id, chunk.getCountOfSequences()});
      }
   }

   for (uint32_t i = 0, limit = partitions.size(); i < limit; ++i) {
      const auto& part = partitions[i];
      for (uint32_t j = 0, limit2 = part.getChunks().size(); j < limit2; ++j) {
         const auto& chunk = part.getChunks()[j];
         for (const auto& pango : chunk.getPangoLineages()) {
            pango_to_chunk[pango] = {i, j, chunk.getCountOfSequences()};
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
      if (!pango_lineage_counts.pango_lineage_counts.empty()) {
         auto it = pango_lineage_counts.pango_lineage_counts.begin();
         Chunk chunk{it->pango_lineage, it->count_of_sequences};
         it++;
         for (; it != pango_lineage_counts.pango_lineage_counts.end(); ++it) {
            chunk.addChunk({it->pango_lineage, it->count_of_sequences});
         }
         partitions.emplace_back(std::vector<Chunk>{{chunk}});
      }
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

Chunk::Chunk(std::string_view lineage, uint32_t count)
    : prefix(lineage),
      count_of_sequences(count),
      offset(0),
      pango_lineages({{std::string{lineage}}}) {}

Chunk::Chunk(std::vector<std::string>&& lineages, uint32_t count)
    : count_of_sequences(count),
      offset(0),
      pango_lineages(lineages) {
   if (lineages.empty()) {
      throw std::runtime_error("Empty chunks should be impossible to create by design.");
   }
   std::sort(pango_lineages.begin(), pango_lineages.end());
   prefix = commonPangoPrefix(pango_lineages.front(), pango_lineages.back());
}

void Chunk::addChunk(Chunk&& other) {
   prefix = commonPangoPrefix(prefix, other.getPrefix());
   count_of_sequences += other.count_of_sequences;
   pango_lineages.insert(
      pango_lineages.end(), other.pango_lineages.begin(), other.pango_lineages.end()
   );
}

std::string_view Chunk::getPrefix() const {
   return prefix;
}

uint32_t Chunk::getCountOfSequences() const {
   return count_of_sequences;
}

uint32_t Chunk::getOffset() const {
   return offset;
}

const std::vector<std::string>& Chunk::getPangoLineages() const {
   return pango_lineages;
}

}  // namespace silo::preprocessing

std::size_t std::hash<silo::preprocessing::PartitionChunk>::operator()(
   const silo::preprocessing::PartitionChunk& partition_chunk
) const {
   return hash<uint32_t>()(partition_chunk.partition) +
          (hash<uint32_t>()(partition_chunk.chunk) << 3) +
          (hash<uint32_t>()(partition_chunk.chunk) >> 2);
}

template <>
struct nlohmann::adl_serializer<silo::preprocessing::Chunk> {
   // NOLINTNEXTLINE(readability-identifier-naming)
   static silo::preprocessing::Chunk from_json(const nlohmann::json& js_object) {
      return silo::preprocessing::Chunk{
         js_object["lineages"].template get<std::vector<std::string>>(),
         js_object["count"].template get<uint32_t>()};
   }

   // NOLINTNEXTLINE(readability-identifier-naming)
   static void to_json(nlohmann::json& js_object, silo::preprocessing::Chunk chunk) {
      js_object["lineages"] = chunk.getPangoLineages();
      js_object["count"] = chunk.getCountOfSequences();
   }
};

template <>
struct nlohmann::adl_serializer<silo::preprocessing::Partition> {
   // NOLINTNEXTLINE(readability-identifier-naming)
   static silo::preprocessing::Partition from_json(const nlohmann::json& js_object) {
      return silo::preprocessing::Partition{
         js_object.template get<std::vector<silo::preprocessing::Chunk>>()};
   }

   // NOLINTNEXTLINE(readability-identifier-naming)
   static void to_json(nlohmann::json& js_object, silo::preprocessing::Partition partition) {
      js_object = partition.getChunks();
   }
};

namespace silo::preprocessing {

void Partitions::save(std::ostream& output_file) const {
   const nlohmann::json json(partitions);
   output_file << json.dump(4) << std::endl;
}

Partitions Partitions::load(std::istream& input_file) {
   nlohmann::json json;
   json = nlohmann::json::parse(input_file);
   const std::vector<Partition> partitions = json.get<std::vector<Partition>>();
   return Partitions{partitions};
}
}  // namespace silo::preprocessing
