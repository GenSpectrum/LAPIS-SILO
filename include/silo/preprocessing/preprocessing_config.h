#ifndef SILO_PREPROCESSING_CONFIG_H
#define SILO_PREPROCESSING_CONFIG_H

#include <filesystem>
#include <string>

namespace silo {

struct InputDirectory {
   std::string directory;
};

struct OutputDirectory {
   std::string directory;
};

struct MetadataFilename {
   std::string filename;
};

struct SequenceFilename {
   std::string filename;
};

struct PangoLineageDefinitionFilename {
   std::string filename;
};

struct PartitionFolder {
   std::string folder;
};

struct SortedPartitionFolder {
   std::string folder;
};

struct SerializationFolder {
   std::string folder;
};

struct PreprocessingConfig {
   explicit PreprocessingConfig(
      const InputDirectory& input_directory_ = {"./"},
      const OutputDirectory& output_directory_ = {"./"},
      const MetadataFilename& metadata_filename_ = {"small_metadata_set.tsv"},
      const SequenceFilename& sequence_filename_ = {"small_sequence_set.fasta"},
      const PangoLineageDefinitionFilename& pango_lineage_definition_filename_ =
         {"pango_alias.txt"},
      const PartitionFolder& partition_folder_ = {"partitioned/"},
      const SortedPartitionFolder& sorted_partition_folder_ = {"sorted_partitions/"},
      const SerializationFolder& serialization_folder_ = {"serialized_state/"}
   );

   std::filesystem::path pango_lineage_definition_file;
   std::filesystem::path metadata_file;
   std::filesystem::path sequence_file;
   std::filesystem::path partition_folder;
   std::filesystem::path sorted_partition_folder;
   std::filesystem::path serialization_folder;
};

}  // namespace silo

#endif  // SILO_PREPROCESSING_CONFIG_H
