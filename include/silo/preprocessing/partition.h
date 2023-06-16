#ifndef SILO_PARTITION_H
#define SILO_PARTITION_H

#include <string>
#include <vector>

namespace silo::preprocessing {

class PangoLineageCounts;

struct Chunk {
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, [[maybe_unused]] const unsigned int version) {
      archive& prefix;
      archive& count_of_sequences;
      archive& offset;
      archive& pango_lineages;
   }
   std::string prefix;
   uint32_t count_of_sequences;
   uint32_t offset;
   std::vector<std::string> pango_lineages;
};

struct Partition {
   std::string name;
   uint32_t count_of_sequences;
   std::vector<Chunk> chunks;
};

struct Partitions {
   std::vector<Partition> partitions;

   static silo::preprocessing::Partitions load(std::istream& input_file);

   void save(std::ostream& output_file) const;
};

enum Architecture { MAX_PARTITIONS, SINGLE_PARTITION, HYBRID, SINGLE_SINGLE };

void calculateOffsets(Partitions& partitions);

Partitions buildPartitions(const PangoLineageCounts& pango_lineage_counts, Architecture arch);

}  // namespace silo::preprocessing

#endif  // SILO_PARTITION_H
