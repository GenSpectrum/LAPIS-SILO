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
