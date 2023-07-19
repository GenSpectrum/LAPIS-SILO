#ifndef SILO_PARTITION_H
#define SILO_PARTITION_H

#include <cstdint>
#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace boost::serialization {
class access;
}

namespace silo::preprocessing {

class PangoLineageCounts;
class Partition;

class Chunk {
   friend class Partition;

   std::string prefix;
   uint32_t count_of_sequences;
   uint32_t offset;
   std::vector<std::string> pango_lineages;

  public:
   Chunk(std::string_view lineage, uint32_t count);
   Chunk(std::vector<std::string>&& lineages, uint32_t count);

   void addChunk(Chunk&& other);

   std::string_view getPrefix() const;
   uint32_t getCountOfSequences() const;
   uint32_t getOffset() const;
   const std::vector<std::string>& getPangoLineages() const;
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
   explicit Partitions(std::vector<Partition> partitions_);

   void save(std::ostream& output_file) const;

   static Partitions load(std::istream& input_file);

   [[nodiscard]] const std::vector<Partition>& getPartitions() const;

   [[nodiscard]] const std::vector<PartitionChunk>& getPartitionChunks() const;

   [[nodiscard]] const std::unordered_map<std::string, silo::preprocessing::PartitionChunk>&
   getPangoToChunk() const;
};

Partitions buildPartitions(const PangoLineageCounts& pango_lineage_counts, Architecture arch);

}  // namespace silo::preprocessing

template <>
struct std::hash<silo::preprocessing::PartitionChunk> {
   std::size_t operator()(const silo::preprocessing::PartitionChunk& partition_chunk) const;
};

#endif  // SILO_PARTITION_H
