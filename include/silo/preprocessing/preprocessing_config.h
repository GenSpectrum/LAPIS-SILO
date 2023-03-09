#ifndef SILO_PREPROCESSING_CONFIG_H
#define SILO_PREPROCESSING_CONFIG_H

#include <filesystem>
#include <string>

namespace silo {
struct PreprocessingConfig {
   PreprocessingConfig(
      const std::string& input_directory_ = "./",
      const std::string& output_directory_ = "./",
      const std::string& metadata_filename_ = "minimal_metadata_set.tsv",
      const std::string& sequence_filename_ = "minimal_sequence_set.fasta",
      const std::string& pango_lineage_definition_filename_ = "pango_alias.txt",
      const std::string& partition_folder_ = "partitioned/",
      const std::string& serialization_folder_ = "serialized_state/"
   );

   std::filesystem::path pango_lineage_definition_file;
   std::filesystem::path metadata_file;
   std::filesystem::path sequence_file;
   std::filesystem::path partition_folder;
   std::filesystem::path serialization_folder;
};
}  // namespace silo

#endif  // SILO_PREPROCESSING_CONFIG_H
