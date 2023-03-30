#ifndef SILO_PARTITION_H
#define SILO_PARTITION_H

#include <string>
#include <vector>

namespace silo {

class PangoLineageCounts;

namespace preprocessing {

struct Chunk {
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, [[maybe_unused]] const unsigned int version) {
      archive& prefix;
      archive& count;
      archive& offset;
      archive& pango_lineages;
   }
   std::string prefix;
   uint32_t count;
   uint32_t offset;
   std::vector<std::string> pango_lineages;
};

struct Partition {
   std::string name;
   uint32_t count;
   std::vector<Chunk> chunks;
};

struct Partitions {
   std::vector<Partition> partitions;

   static silo::preprocessing::Partitions load(std::istream& input_file);

   void save(std::ostream& output_file);
};

enum Architecture { MAX_PARTITIONS, SINGLE_PARTITION, HYBRID, SINGLE_SINGLE };

Partitions buildPartitions(PangoLineageCounts pango_lineage_counts, Architecture arch);

}  // namespace preprocessing
}  // namespace silo

#endif  // SILO_PARTITION_H
