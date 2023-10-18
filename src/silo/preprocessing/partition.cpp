#include "silo/preprocessing/partition.h"

#include <algorithm>
#include <istream>
#include <iterator>
#include <list>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/common/pango_lineage.h"
#include "silo/config/database_config.h"
#include "silo/preprocessing/metadata.h"
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
std::vector<silo::preprocessing::LineageGroup> mergePangosToLineageGroups(
   const std::vector<PangoLineageCount>& pango_lineage_counts,
   // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
   uint32_t target_size,
   uint32_t min_size
) {
   // Initialize chunks such that every chunk is just a pango_lineage
   SPDLOG_TRACE(
      "Initialize {} chunks such that every chunk is just a pango_lineage",
      pango_lineage_counts.size()
   );
   std::list<LineageGroup> lineage_groups;
   for (const auto& [lineage, count] : pango_lineage_counts) {
      lineage_groups.emplace_back(lineage, count);
   }
   // We want to prioritise merges more closely related chunks.
   // Therefore, we first merge the chunks, with longer matching prefixes.
   // Precalculate the longest a prefix can be (which is the max length of lineages)
   const uint32_t max_len =
      std::max_element(
         pango_lineage_counts.begin(),
         pango_lineage_counts.end(),
         [](const PangoLineageCount& lhs, const PangoLineageCount& rhs) {
            return lhs.pango_lineage.value.size() < rhs.pango_lineage.value.size();
         }
      )->pango_lineage.value.size();
   SPDLOG_TRACE("Precalculated longest prefix: {}", max_len);
   for (uint32_t len = max_len; len > 0; len--) {
      SPDLOG_TRACE(
         "Merging chunks with prefix length: {}. Leftover chunks: {}", len, lineage_groups.size()
      );
      for (auto it = lineage_groups.begin();
           it != lineage_groups.end() && std::next(it) != lineage_groups.end();) {
         auto&& [pango1, pango2] = std::tie(*it, *std::next(it));
         const std::string common_prefix =
            commonPangoPrefix(pango1.getPrefix(), pango2.getPrefix());
         // We only look at possible merges with a common_prefix length of #len
         const bool one_chunk_is_very_small =
            pango1.getCountOfSequences() < min_size || pango2.getCountOfSequences() < min_size;
         const bool both_chunks_still_want_to_grow = pango1.getCountOfSequences() < target_size &&
                                                     pango2.getCountOfSequences() < target_size;
         if (common_prefix.size() == len && (one_chunk_is_very_small || both_chunks_still_want_to_grow)) {
            SPDLOG_TRACE(
               "Merging chunks {} and {} with common prefix {}",
               pango1.getPrefix(),
               pango2.getPrefix(),
               common_prefix
            );
            pango2.addLineageGroup(std::move(pango1));

            // We merged pango1 into pango2 -> Now delete pango1
            // Do not need to increment, because erase will make it automatically point to next
            // element
            it = lineage_groups.erase(it);
         } else {
            ++it;
         }
      }
      SPDLOG_TRACE(
         "Finished merging chunks with prefix length: {}. Leftover chunks: {}",
         len,
         lineage_groups.size()
      );
   }

   std::vector<LineageGroup> return_vector;
   std::copy(
      std::begin(lineage_groups), std::end(lineage_groups), std::back_inserter(return_vector)
   );
   return return_vector;
}

silo::preprocessing::Partition::Partition(std::vector<PartitionChunk>&& chunks_)
    : chunks(std::move(chunks_)) {
   uint32_t running_total = 0;
   for (PartitionChunk& chunk : chunks) {
      running_total += chunk.size;
   }
   sequence_count = running_total;
}

silo::preprocessing::Partition::Partition(
   uint32_t partition_id,
   std::vector<LineageGroup>&& lineage_groups
) {
   uint32_t running_total = 0;
   uint32_t chunk_id = 0;
   for (LineageGroup& group : lineage_groups) {
      chunks.push_back(PartitionChunk{
         partition_id, chunk_id, group.getCountOfSequences(), running_total});
      chunk_id++;
      running_total += group.getCountOfSequences();
   }
   sequence_count = running_total;
}

const std::vector<PartitionChunk>& silo::preprocessing::Partition::getPartitionChunks() const {
   return chunks;
}

uint32_t Partition::getSequenceCount() const {
   return sequence_count;
}

silo::preprocessing::Partitions::Partitions() = default;

silo::preprocessing::Partitions::Partitions(std::vector<Partition> partitions_)
    : partitions(std::move(partitions_)) {
   for (uint32_t part_id = 0, limit = partitions.size(); part_id < limit; ++part_id) {
      const auto& part = partitions[part_id];
      for (uint32_t chunk_id = 0, limit2 = part.getPartitionChunks().size(); chunk_id < limit2;
           ++chunk_id) {
         all_partition_chunks.emplace_back(part.getPartitionChunks()[chunk_id]);
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
   SPDLOG_TRACE("Total count of sequences: {}", total_count_of_sequences);
   constexpr int TARGET_SIZE_REDUCTION = 100;
   constexpr int MIN_SIZE_REDUCTION = 200;

   if (arch == Architecture::MAX_PARTITIONS) {
      SPDLOG_INFO("Building partitions with architecture MAX_PARTITIONS");
      uint32_t partition_id = 0;
      for (auto& lineage_group : mergePangosToLineageGroups(
              pango_lineage_counts.pango_lineage_counts,
              total_count_of_sequences / 100,
              total_count_of_sequences / 200
           )) {
         partitions.emplace_back(partition_id, std::vector<LineageGroup>{{lineage_group}});
         partition_id++;
      }
   } else if (arch == Architecture::SINGLE_PARTITION) {
      // Merge pango_lineages, such that chunks are not get very small
      SPDLOG_INFO("Building partitions with architecture SINGLE_PARTITION");
      std::vector<LineageGroup> lineage_groups = mergePangosToLineageGroups(
         pango_lineage_counts.pango_lineage_counts,
         total_count_of_sequences / TARGET_SIZE_REDUCTION,
         total_count_of_sequences / MIN_SIZE_REDUCTION
      );

      partitions.emplace_back(0, std::move(lineage_groups));
   } else if (arch == Architecture::SINGLE_SINGLE) {
      // Merge pango_lineages, such that all lineages are in one chunk
      SPDLOG_INFO("Building partitions with architecture SINGLE_SINGLE");
      if (!pango_lineage_counts.pango_lineage_counts.empty()) {
         auto current = pango_lineage_counts.pango_lineage_counts.begin();
         LineageGroup lineage_group{current->pango_lineage, current->count_of_sequences};
         current++;
         for (; current != pango_lineage_counts.pango_lineage_counts.end(); ++current) {
            lineage_group.addLineageGroup({current->pango_lineage, current->count_of_sequences});
         }
         partitions.emplace_back(0, std::vector<LineageGroup>{{lineage_group}});
      }
   }
   return Partitions{partitions};
}

Partitions createSingletonPartitions(
   const std::filesystem::path& metadata_path,
   const silo::config::DatabaseConfig& database_config
) {
   const uint32_t count_of_sequences = silo::preprocessing::MetadataReader(metadata_path)
                                          .getColumn(database_config.schema.primary_key)
                                          .size();

   const preprocessing::PartitionChunk singleton_chunk{0, 0, count_of_sequences, 0};
   const preprocessing::Partition singleton_partition({singleton_chunk});

   return preprocessing::Partitions({singleton_partition});
}

const std::vector<Partition>& Partitions::getPartitions() const {
   return partitions;
}

const std::vector<PartitionChunk>& Partitions::getAllPartitionChunks() const {
   return all_partition_chunks;
}

bool PartitionChunk::operator==(const PartitionChunk& other) const {
   return partition == other.partition && chunk == other.chunk && size == other.size;
}

LineageGroup::LineageGroup(silo::common::UnaliasedPangoLineage lineage, uint32_t count)
    : prefix(lineage.value),
      count_of_sequences(count),
      pango_lineages({{std::move(lineage)}}) {}

LineageGroup::LineageGroup(
   std::vector<silo::common::UnaliasedPangoLineage>&& lineages,
   uint32_t count
)
    : count_of_sequences(count),
      pango_lineages(lineages) {
   if (lineages.empty()) {
      throw std::runtime_error("Empty chunks should be impossible to create by design.");
   }
   std::sort(pango_lineages.begin(), pango_lineages.end());
   prefix = commonPangoPrefix(pango_lineages.front().value, pango_lineages.back().value);
}

void LineageGroup::addLineageGroup(LineageGroup&& other) {
   prefix = commonPangoPrefix(prefix, other.getPrefix());
   count_of_sequences += other.count_of_sequences;
   // Add all pango lineages but keep invariant of pango lineages being sorted
   auto copy_of_my_lineages = std::move(pango_lineages);
   pango_lineages.clear();
   pango_lineages.resize(copy_of_my_lineages.size() + other.pango_lineages.size());
   std::merge(
      copy_of_my_lineages.begin(),
      copy_of_my_lineages.end(),
      other.pango_lineages.begin(),
      other.pango_lineages.end(),
      pango_lineages.begin()
   );
}

std::string_view LineageGroup::getPrefix() const {
   return prefix;
}

uint32_t LineageGroup::getCountOfSequences() const {
   return count_of_sequences;
}

const std::vector<silo::common::UnaliasedPangoLineage>& LineageGroup::getPangoLineages() const {
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
struct nlohmann::adl_serializer<silo::preprocessing::PartitionChunk> {
   // NOLINTNEXTLINE(readability-identifier-naming)
   static silo::preprocessing::PartitionChunk from_json(const nlohmann::json& js_object) {
      return silo::preprocessing::PartitionChunk{
         js_object["partition"].template get<uint32_t>(),
         js_object["chunk"].template get<uint32_t>(),
         js_object["size"].template get<uint32_t>(),
         js_object["offset"].template get<uint32_t>()};
   }

   // NOLINTNEXTLINE(readability-identifier-naming)
   static void to_json(
      nlohmann::json& js_object,
      const silo::preprocessing::PartitionChunk& chunk
   ) {
      js_object["partition"] = chunk.partition;
      js_object["chunk"] = chunk.chunk;
      js_object["size"] = chunk.size;
      js_object["offset"] = chunk.offset;
   }
};

template <>
struct nlohmann::adl_serializer<silo::preprocessing::Partition> {
   // NOLINTNEXTLINE(readability-identifier-naming)
   static silo::preprocessing::Partition from_json(const nlohmann::json& js_object) {
      return silo::preprocessing::Partition{
         js_object.template get<std::vector<silo::preprocessing::PartitionChunk>>()};
   }

   // NOLINTNEXTLINE(readability-identifier-naming)
   static void to_json(nlohmann::json& js_object, const silo::preprocessing::Partition& partition) {
      js_object = partition.getPartitionChunks();
   }
};

namespace silo::preprocessing {

void Partitions::save(std::ostream& output_file) const {
   const nlohmann::json json(partitions);
   output_file << json.dump(4);
}

Partitions Partitions::load(std::istream& input_file) {
   nlohmann::json json;
   json = nlohmann::json::parse(input_file);
   const std::vector<Partition> partitions = json.get<std::vector<Partition>>();
   return Partitions{partitions};
}
}  // namespace silo::preprocessing
