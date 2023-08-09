#include "silo/preprocessing/preprocessing_config_reader.h"

#include <string>

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include "silo/preprocessing/preprocessing_config.h"

using silo::preprocessing::OptionalPreprocessingConfig;

namespace YAML {

std::optional<std::string> extractStringIfPresent(const Node& node, const std::string& key) {
   if (node[key]) {
      return node[key].as<std::string>();
   }
   return std::nullopt;
}

template <>
struct convert<OptionalPreprocessingConfig> {
   static bool decode(const Node& node, OptionalPreprocessingConfig& config) {
      config = OptionalPreprocessingConfig{
         extractStringIfPresent(node, "inputDirectory"),
         extractStringIfPresent(node, "outputDirectory"),
         extractStringIfPresent(node, "intermediateResultsDirectory"),
         extractStringIfPresent(node, "metadataFilename"),
         extractStringIfPresent(node, "pangoLineageDefinitionFilename"),
         extractStringIfPresent(node, "partitionsFolder"),
         extractStringIfPresent(node, "sortedPartitionsFolder"),
         extractStringIfPresent(node, "referenceGenomeFilename"),
         extractStringIfPresent(node, "nucleotideSequencePrefix"),
         extractStringIfPresent(node, "genePrefix")};

      return true;
   }
};
}  // namespace YAML

namespace silo::preprocessing {

OptionalPreprocessingConfig PreprocessingConfigReader::readConfig(
   const std::filesystem::path& config_path
) const {
   SPDLOG_INFO("Reading preprocessing config from {}", config_path.string());
   return YAML::LoadFile(config_path.string()).as<OptionalPreprocessingConfig>();
}

PreprocessingConfig OptionalPreprocessingConfig::mergeValuesFromOrDefault(
   const silo::preprocessing::OptionalPreprocessingConfig& other
) const {
   return PreprocessingConfig(
      InputDirectory{input_directory.value_or(
         other.input_directory.value_or(silo::preprocessing::DEFAULT_INPUT_DIRECTORY.directory)
      )},
      IntermediateResultsDirectory{
         intermediate_results_directory.value_or(other.intermediate_results_directory.value_or(
            silo::preprocessing::DEFAULT_INTERMEDIATE_RESULTS_DIRECTORY.directory
         ))},
      OutputDirectory{output_directory.value_or(
         other.output_directory.value_or(silo::preprocessing::DEFAULT_OUTPUT_DIRECTORY.directory)
      )},
      MetadataFilename{metadata_file.value_or(
         other.metadata_file.value_or(silo::preprocessing::DEFAULT_METADATA_FILENAME.filename)
      )},
      PangoLineageDefinitionFilename{
         pango_lineage_definition_file.has_value() ? pango_lineage_definition_file
                                                   : other.pango_lineage_definition_file},
      PartitionsFolder{partition_folder.value_or(
         other.partition_folder.value_or(silo::preprocessing::DEFAULT_PARTITIONS_FOLDER.folder)
      )},
      SortedPartitionsFolder{
         sorted_partition_folder.value_or(other.sorted_partition_folder.value_or(
            silo::preprocessing::DEFAULT_SORTED_PARTITIONS_FOLDER.folder
         ))},
      ReferenceGenomeFilename{reference_genome_file.value_or(other.reference_genome_file.value_or(
         silo::preprocessing::DEFAULT_REFERENCE_GENOME_FILENAME.filename
      ))},
      NucleotideSequencePrefix{
         nucleotide_sequence_prefix.value_or(other.nucleotide_sequence_prefix.value_or(
            silo::preprocessing::DEFAULT_NUCLEOTIDE_SEQUENCE_PREFIX.prefix
         ))},
      GenePrefix{gene_prefix.value_or(
         other.gene_prefix.value_or(silo::preprocessing::DEFAULT_GENE_PREFIX.prefix)
      )}
   );
}

}  // namespace silo::preprocessing