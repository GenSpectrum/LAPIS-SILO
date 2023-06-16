#include "silo/preprocessing/preprocessing_config.h"

#include <iostream>

namespace silo::preprocessing {

std::filesystem::path createPath(
   const std::filesystem::path& directory,
   const std::string& filename
) {
   auto return_path = directory;
   return_path += filename;
   if (!std::filesystem::exists(return_path)) {
      throw std::filesystem::filesystem_error(
         return_path.relative_path().string() + " does not exist", std::error_code()
      );
   }
   return return_path;
}

std::filesystem::path createOutputPath(
   const std::filesystem::path& output_directory,
   const std::string& folder
) {
   auto return_path = output_directory;
   return_path += folder;
   if (!std::filesystem::exists(return_path)) {
      std::filesystem::create_directory(return_path);
   }
   return return_path;
}

PreprocessingConfig::PreprocessingConfig() = default;

PreprocessingConfig::PreprocessingConfig(
   const InputDirectory& input_directory_,
   const OutputDirectory& output_directory_,
   const MetadataFilename& metadata_filename_,
   const SequenceFilename& sequence_filename_,
   const PangoLineageDefinitionFilename& pango_lineage_definition_filename_,
   const PartitionsFolder& partition_folder_,
   const SortedPartitionsFolder& sorted_partition_folder_,
   const SerializedStateFolder& serialization_folder_
) {
   input_directory = input_directory_.directory;
   if (!std::filesystem::exists(input_directory)) {
      throw std::filesystem::filesystem_error(
         input_directory.string() + " does not exist", std::error_code()
      );
   }

   metadata_file = createPath(input_directory, metadata_filename_.filename);
   pango_lineage_definition_file =
      createPath(input_directory, pango_lineage_definition_filename_.filename);
   sequence_file = createPath(input_directory, sequence_filename_.filename);

   const std::filesystem::path output_directory(output_directory_.directory);
   if (!std::filesystem::exists(output_directory_.directory)) {
      std::filesystem::create_directory(output_directory_.directory);
   }

   partition_folder = createOutputPath(output_directory, partition_folder_.folder);
   sorted_partition_folder = createOutputPath(output_directory, sorted_partition_folder_.folder);
   serialization_folder = createOutputPath(output_directory, serialization_folder_.folder);
}
}  // namespace silo::preprocessing