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

class LineageGroup {
   friend class Partition;

   std::string prefix;
   uint32_t count_of_sequences;
   std::vector<common::UnaliasedPangoLineage> pango_lineages;

   LineageGroup() = default;

  public:
   LineageGroup(silo::common::UnaliasedPangoLineage lineage, uint32_t count);
   LineageGroup(std::vector<silo::common::UnaliasedPangoLineage>&& lineages, uint32_t count);

   void addLineageGroup(LineageGroup&& other);

   std::string_view getPrefix() const;
   uint32_t getCountOfSequences() const;
   const std::vector<silo::common::UnaliasedPangoLineage>& getPangoLineages() const;
};

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

   explicit Partition(uint32_t partition_id, std::vector<LineageGroup>&& lineage_groups);

   [[nodiscard]] const std::vector<PartitionChunk>& getPartitionChunks() const;

   [[nodiscard]] uint32_t getSequenceCount() const;
};

enum Architecture { MAX_PARTITIONS, SINGLE_PARTITION, SINGLE_SINGLE };

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

   [[nodiscard]] const std::vector<PartitionChunk>& getAllPartitionChunks() const;
};

Partitions createSingletonPartitions(
   const std::filesystem::path& metadata_path,
   const silo::config::DatabaseConfig& database_config
);

}  // namespace silo::preprocessing

template <>
struct std::hash<silo::preprocessing::PartitionChunk> {
   std::size_t operator()(const silo::preprocessing::PartitionChunk& partition_chunk) const;
};
