#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <iosfwd>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/serialization/access.hpp>
#include <nlohmann/json_fwd.hpp>

namespace boost::serialization {
class access;
}
namespace silo::common {
class UnaliasedPangoLineage;
}
namespace silo::config {
class DatabaseConfig;
}

namespace silo::preprocessing {

class PangoLineageCounts;
class Partition;

class Chunk {
   friend class Partition;
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive & prefix;
      archive & count_of_sequences;
      archive & offset;
      archive & pango_lineages;
      // clang-format on
   }

   std::string prefix;
   uint32_t count_of_sequences;
   uint32_t offset;
   std::vector<common::UnaliasedPangoLineage> pango_lineages;

   Chunk() = default;

  public:
   Chunk(silo::common::UnaliasedPangoLineage lineage, uint32_t count);
   Chunk(std::vector<silo::common::UnaliasedPangoLineage>&& lineages, uint32_t count);

   void addChunk(Chunk&& other);

   std::string_view getPrefix() const;
   uint32_t getCountOfSequences() const;
   uint32_t getOffset() const;
   const std::vector<silo::common::UnaliasedPangoLineage>& getPangoLineages() const;
};

class Partition {
   uint32_t sequence_count;
   std::vector<Chunk> chunks;

  public:
   explicit Partition(std::vector<Chunk>&& chunks);

   [[nodiscard]] const std::vector<Chunk>& getChunks() const;

   [[nodiscard]] uint32_t getSequenceCount() const;
};

struct PartitionChunk {
   uint32_t partition;
   uint32_t chunk;
   uint32_t size;

   bool operator==(const PartitionChunk& other) const;
};

enum Architecture { MAX_PARTITIONS, SINGLE_PARTITION, SINGLE_SINGLE };

class Partitions {
   std::vector<Partition> partitions;

   // Flat map of the counts and sizes of the partitions and containing chunks
   std::vector<PartitionChunk> partition_chunks;

   // Mapping all pango lineages to the chunk they are contained in
   std::unordered_map<std::string, silo::preprocessing::PartitionChunk> pango_to_chunk;

  public:
   Partitions();

   explicit Partitions(std::vector<Partition> partitions_);

   void save(std::ostream& output_file) const;

   static Partitions load(std::istream& input_file);

   [[nodiscard]] const std::vector<Partition>& getPartitions() const;

   [[nodiscard]] const std::vector<PartitionChunk>& getPartitionChunks() const;

   [[nodiscard]] const std::unordered_map<std::string, silo::preprocessing::PartitionChunk>&
   getPangoToChunk() const;
};

Partitions buildPartitions(const PangoLineageCounts& pango_lineage_counts, Architecture arch);

Partitions createSingletonPartitions(
   const std::filesystem::path& metadata_path,
   const silo::config::DatabaseConfig& database_config
);

}  // namespace silo::preprocessing

template <>
struct std::hash<silo::preprocessing::PartitionChunk> {
   std::size_t operator()(const silo::preprocessing::PartitionChunk& partition_chunk) const;
};
