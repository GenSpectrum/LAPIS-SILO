#include "silo/preprocessing/preprocessing_config.h"
#include <iostream>

namespace silo {

std::filesystem::path createPath(const std::filesystem::path& directory, const std::string& filename) {
   auto return_path = directory;
   return_path += filename;
   if (!std::filesystem::exists(return_path)) {
      std::cerr << return_path.relative_path() << " does not exist" << std::endl;
      throw std::filesystem::filesystem_error(std::string(return_path.relative_path()) + " does not exist", std::error_code());
   }
   return return_path;
}

std::filesystem::path createOutputPath(const std::filesystem::path& output_directory, const std::string& folder) {
   auto return_path = output_directory;
   return_path += folder;
   if (!std::filesystem::exists(return_path)) {
      std::filesystem::create_directory(return_path);
   }
   return return_path;
}

PreprocessingConfig::PreprocessingConfig(
   const std::string& input_directory_,
   const std::string& output_directory_,
   const std::string& metadata_filename_,
   const std::string& sequence_filename_,
   const std::string& pango_lineage_definition_filename_,
   const std::string& partition_folder_,
   const std::string& serialization_folder_) {
   std::filesystem::path input_directory(input_directory_);
   if (!std::filesystem::exists(input_directory)) {
      throw std::filesystem::filesystem_error(std::string(input_directory) + " does not exist", std::error_code());
   }

   metadata_file = createPath(input_directory, metadata_filename_);
   pango_lineage_definition_file = createPath(input_directory, pango_lineage_definition_filename_);
   sequence_file = createPath(input_directory, sequence_filename_);

   std::filesystem::path output_directory(output_directory_);
   if (!std::filesystem::exists(output_directory_)) {
      std::filesystem::create_directory(output_directory_);
   }

   partition_folder = createOutputPath(output_directory, partition_folder_);
   serialization_folder = createOutputPath(output_directory, serialization_folder_);
}
}