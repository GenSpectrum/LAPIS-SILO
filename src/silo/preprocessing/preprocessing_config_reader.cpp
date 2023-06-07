#include "silo/preprocessing/preprocessing_config_reader.h"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include "silo/preprocessing/preprocessing_config.h"

using silo::preprocessing::InputDirectory;
using silo::preprocessing::MetadataFilename;
using silo::preprocessing::OutputDirectory;
using silo::preprocessing::PangoLineageDefinitionFilename;
using silo::preprocessing::PartitionFolder;
using silo::preprocessing::PreprocessingConfig;
using silo::preprocessing::SequenceFilename;
using silo::preprocessing::SerializationFolder;
using silo::preprocessing::SortedPartitionFolder;

namespace YAML {
template <>
struct convert<PreprocessingConfig> {
   static bool decode(const Node& node, PreprocessingConfig& config) {
      const InputDirectory input_directory{node["inputDirectory"].as<std::string>()};
      const OutputDirectory output_directory{node["outputDirectory"].as<std::string>()};
      const MetadataFilename metadata_filename{node["metadataFilename"].as<std::string>()};
      const SequenceFilename sequence_filename{node["sequenceFilename"].as<std::string>()};
      const PangoLineageDefinitionFilename pango_lineage_definition_filename{
         node["pangoLineageDefinitionFilename"].as<std::string>()};

      const PartitionFolder partition_folder{node["partitionFolder"].as<std::string>()};
      const SortedPartitionFolder sorted_partition_folder{
         node["sortedPartitionFolder"].as<std::string>()};
      const SerializationFolder serialization_folder{node["serializationFolder"].as<std::string>()};

      config = PreprocessingConfig(
         input_directory,
         output_directory,
         metadata_filename,
         sequence_filename,
         pango_lineage_definition_filename,
         partition_folder,
         sorted_partition_folder,
         serialization_folder
      );

      return true;
   }
};
}  // namespace YAML

namespace silo::preprocessing {
PreprocessingConfig PreprocessingConfigReader::readConfig(const std::filesystem::path& config_path
) const {
   SPDLOG_INFO("Reading preprocessing config from {}", config_path.string());
   return YAML::LoadFile(config_path.string()).as<PreprocessingConfig>();
}
}  // namespace silo::preprocessing