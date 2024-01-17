#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <iosfwd>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "silo/common/pango_lineage.h"

namespace boost::serialization {
class access;
}
namespace silo::config {
class DatabaseConfig;
}

namespace silo::preprocessing {

struct PartitionChunk {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive & partition;
      archive & chunk;
      archive & size;
      archive & offset;
      // clang-format on
   }

   uint32_t partition;
   uint32_t chunk;
   uint32_t size;
   uint32_t offset;

   bool operator==(const PartitionChunk& other) const;
};

class Partition {
   uint32_t sequence_count;
   std::vector<PartitionChunk> chunks;

  public:
   explicit Partition(std::vector<PartitionChunk>&& chunks);

   [[nodiscard]] const std::vector<PartitionChunk>& getPartitionChunks() const;
};

class Partitions {
   std::vector<Partition> partitions;

   // Flat map of the counts and sizes of the partitions and containing chunks
   std::vector<PartitionChunk> all_partition_chunks;

  public:
   Partitions();

   explicit Partitions(std::vector<Partition> partitions_);

   void save(std::ostream& output_file) const;

   static Partitions load(std::istream& input_file);

   [[nodiscard]] const std::vector<Partition>& getPartitions() const;
};

}  // namespace silo::preprocessing

template <>
struct std::hash<silo::preprocessing::PartitionChunk> {
   std::size_t operator()(const silo::preprocessing::PartitionChunk& partition_chunk) const;
};
