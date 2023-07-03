#ifndef SILO_PREPROCESSING_CONFIG_H
#define SILO_PREPROCESSING_CONFIG_H

#include <filesystem>
#include <string>

namespace silo::preprocessing {

struct InputDirectory {
   std::string directory;
};

struct OutputDirectory {
   std::string directory;
};

struct MetadataFilename {
   std::string filename;
};

struct PangoLineageDefinitionFilename {
   std::string filename;
};

struct PartitionsFolder {
   std::string folder;
};

struct SortedPartitionsFolder {
   std::string folder;
};

struct SerializedStateFolder {
   std::string folder;
};

struct ReferenceGenomeFilename {
   std::string filename;
};

struct PreprocessingConfig {
   explicit PreprocessingConfig(
      const InputDirectory& input_directory_,
      const OutputDirectory& output_directory_,
      const MetadataFilename& metadata_filename_,
      const PangoLineageDefinitionFilename& pango_lineage_definition_filename_,
      const PartitionsFolder& partition_folder_,
      const SortedPartitionsFolder& sorted_partition_folder_,
      const SerializedStateFolder& serialization_folder_,
      const ReferenceGenomeFilename& reference_genome_filename_
   );

   explicit PreprocessingConfig();

   std::filesystem::path input_directory;
   std::filesystem::path pango_lineage_definition_file;
   std::filesystem::path metadata_file;
   std::filesystem::path partition_folder;
   std::filesystem::path sorted_partition_folder;
   std::filesystem::path serialization_folder;
   std::filesystem::path reference_genome_file;
};

std::filesystem::path createPath(
   const std::filesystem::path& directory,
   const std::string& filename
);

}  // namespace silo::preprocessing

#endif  // SILO_PREPROCESSING_CONFIG_H
