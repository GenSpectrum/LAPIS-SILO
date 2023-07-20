#include "silo/preprocessing/preprocessing_config_reader.h"

#include <string>

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include "silo/preprocessing/preprocessing_config.h"

using silo::preprocessing::GenePrefix;
using silo::preprocessing::InputDirectory;
using silo::preprocessing::MetadataFilename;
using silo::preprocessing::NucleotideSequencePrefix;
using silo::preprocessing::OutputDirectory;
using silo::preprocessing::PangoLineageDefinitionFilename;
using silo::preprocessing::PartitionsFolder;
using silo::preprocessing::PreprocessingConfig;
using silo::preprocessing::ReferenceGenomeFilename;
using silo::preprocessing::SerializedStateFolder;
using silo::preprocessing::SortedPartitionsFolder;

namespace YAML {

std::string nucleotideSequencePrefix(const Node& node) {
   if (node["nucleotideSequencePrefix"]) {
      return node["nucleotideSequencePrefix"].as<std::string>();
   }
   SPDLOG_DEBUG("nucleotideSequencePrefix not found in config file. Using default value: nuc_");
   return "nuc_";
}
std::string genePrefix(const Node& node) {
   if (node["genePrefix"]) {
      return node["genePrefix"].as<std::string>();
   }
   SPDLOG_DEBUG("genePrefix not found in config file. Using default value: gene_");
   return "gene_";
}

std::string partitionsFolderName(const Node& node) {
   if (node["partitionsFolder"]) {
      return node["partitionsFolder"].as<std::string>();
   }
   SPDLOG_DEBUG("partitionFolder not found in config file. Using default value: partitions/");
   return "partitions/";
}

std::string sortedPartitionsFolderName(const Node& node) {
   if (node["sortedPartitionsFolder"]) {
      return node["sortedPartitionsFolder"].as<std::string>();
   }
   SPDLOG_DEBUG(
      "sortedPartitionFolder not found in config file. Using default value: partitions_sorted/"
   );
   return "partitions_sorted/";
}

std::string serializedStateFolderName(const Node& node) {
   if (node["serializedStateFolder"]) {
      return node["serializedStateFolder"].as<std::string>();
   }
   SPDLOG_DEBUG(
      "serializationFolder not found in config file. Using default value: serialized_state/"
   );
   return "serialized_state/";
}

template <>
struct convert<PreprocessingConfig> {
   static bool decode(const Node& node, PreprocessingConfig& config) {
      const InputDirectory input_directory{node["inputDirectory"].as<std::string>()};
      const OutputDirectory output_directory{node["outputDirectory"].as<std::string>()};
      const MetadataFilename metadata_filename{node["metadataFilename"].as<std::string>()};
      const PangoLineageDefinitionFilename pango_lineage_definition_filename{
         node["pangoLineageDefinitionFilename"].as<std::string>()};
      const ReferenceGenomeFilename reference_genome_filename{
         node["referenceGenomeFilename"].as<std::string>()};

      const PartitionsFolder partitions_folder{partitionsFolderName(node)};
      const SortedPartitionsFolder sorted_partitions_folder{sortedPartitionsFolderName(node)};
      const SerializedStateFolder serialized_state_folder{serializedStateFolderName(node)};
      const NucleotideSequencePrefix nucleotide_sequence_prefix{nucleotideSequencePrefix(node)};
      const GenePrefix gene_prefix{genePrefix(node)};

      config = PreprocessingConfig(
         input_directory,
         output_directory,
         metadata_filename,
         pango_lineage_definition_filename,
         partitions_folder,
         sorted_partitions_folder,
         serialized_state_folder,
         reference_genome_filename,
         nucleotide_sequence_prefix,
         gene_prefix
      );

      SPDLOG_TRACE("Resulting preprocessing config: {}", config);

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