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

namespace silo::preprocessing {

silo::preprocessing::Partition::Partition(std::vector<PartitionChunk>&& chunks_)
    : chunks(std::move(chunks_)) {
   uint32_t running_total = 0;
   for (PartitionChunk& chunk : chunks) {
      running_total += chunk.size;
   }
   sequence_count = running_total;
}

const std::vector<PartitionChunk>& silo::preprocessing::Partition::getPartitionChunks() const {
   return chunks;
}

silo::preprocessing::Partitions::Partitions() = default;

silo::preprocessing::Partitions::Partitions(std::vector<Partition> partitions_)
    : partitions(std::move(partitions_)) {
   for (const auto& partition : partitions) {
      for (const auto& chunk : partition.getPartitionChunks()) {
         all_partition_chunks.emplace_back(chunk);
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
