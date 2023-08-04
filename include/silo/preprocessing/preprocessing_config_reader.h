#ifndef SILO_INCLUDE_SILO_API_PREPROCESSING_CONFIG_READER_H_
#define SILO_INCLUDE_SILO_API_PREPROCESSING_CONFIG_READER_H_

#include <filesystem>
#include <optional>

namespace silo::preprocessing {
struct PreprocessingConfig;

struct OptionalPreprocessingConfig {
   std::optional<std::filesystem::path> input_directory;
   std::optional<std::filesystem::path> output_directory;
   std::optional<std::filesystem::path> metadata_file;
   std::optional<std::filesystem::path> pango_lineage_definition_file;
   std::optional<std::filesystem::path> partition_folder;
   std::optional<std::filesystem::path> sorted_partition_folder;
   std::optional<std::filesystem::path> serialization_folder;
   std::optional<std::filesystem::path> reference_genome_file;
   std::optional<std::string> nucleotide_sequence_prefix;
   std::optional<std::string> gene_prefix;

   PreprocessingConfig mergeValuesFromOrDefault(const OptionalPreprocessingConfig& other) const;
};

class PreprocessingConfigReader {
  public:
   virtual OptionalPreprocessingConfig readConfig(const std::filesystem::path& config_path) const;
};
}  // namespace silo::preprocessing

#endif  // SILO_INCLUDE_SILO_API_PREPROCESSING_CONFIG_READER_H_
