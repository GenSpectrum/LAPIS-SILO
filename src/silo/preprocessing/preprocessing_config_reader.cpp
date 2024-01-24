#include "silo/preprocessing/preprocessing_config_reader.h"

#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_exception.h"

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
         extractStringIfPresent(node, "preprocessingDatabaseLocation"),
         extractStringIfPresent(node, "ndjsonInputFilename"),
         extractStringIfPresent(node, "metadataFilename"),
         extractStringIfPresent(node, "pangoLineageDefinitionFilename"),
         extractStringIfPresent(node, "referenceGenomeFilename"),
         extractStringIfPresent(node, "nucleotideSequencePrefix"),
         extractStringIfPresent(node, "genePrefix")
      };

      return true;
   }
};
}  // namespace YAML

namespace silo::preprocessing {

OptionalPreprocessingConfig PreprocessingConfigReader::readConfig(
   const std::filesystem::path& config_path
) const {
   SPDLOG_INFO("Reading preprocessing config from {}", config_path.string());

   try {
      auto config = YAML::LoadFile(config_path.string()).as<OptionalPreprocessingConfig>();
      if (config.ndjson_input_filename.has_value() && config.metadata_file) {
         throw preprocessing::PreprocessingException(fmt::format(
            "Cannot specify both a ndjsonInputFilename ('{}') and metadataFilename('{}').",
            config.ndjson_input_filename.value().string(),
            config.metadata_file.value().string()
         ));
      }
      return config;
   } catch (const YAML::Exception& e) {
      throw std::runtime_error(
         "Failed to read preprocessing config from " + config_path.string() + ": " +
         std::string(e.what())
      );
   }
}

PreprocessingConfig OptionalPreprocessingConfig::mergeValuesFromOrDefault(
   const silo::preprocessing::OptionalPreprocessingConfig& other
) const {
   return silo::preprocessing::PreprocessingConfig(
      InputDirectory{input_directory.value_or(
         other.input_directory.value_or(silo::preprocessing::DEFAULT_INPUT_DIRECTORY.directory)
      )},
      IntermediateResultsDirectory{
         intermediate_results_directory.value_or(other.intermediate_results_directory.value_or(
            silo::preprocessing::DEFAULT_INTERMEDIATE_RESULTS_DIRECTORY.directory
         ))
      },
      OutputDirectory{output_directory.value_or(
         other.output_directory.value_or(silo::preprocessing::DEFAULT_OUTPUT_DIRECTORY.directory)
      )},
      PreprocessingDatabaseLocation{
         preprocessing_database_location.has_value() ? preprocessing_database_location
                                                     : other.preprocessing_database_location
      },
      NdjsonInputFilename{
         ndjson_input_filename.has_value() ? ndjson_input_filename : other.ndjson_input_filename
      },
      MetadataFilename{metadata_file.value_or(
         other.metadata_file.value_or(silo::preprocessing::DEFAULT_METADATA_FILENAME.filename)
      )},
      PangoLineageDefinitionFilename{
         pango_lineage_definition_file.has_value() ? pango_lineage_definition_file
                                                   : other.pango_lineage_definition_file
      },
      ReferenceGenomeFilename{reference_genome_file.value_or(other.reference_genome_file.value_or(
         silo::preprocessing::DEFAULT_REFERENCE_GENOME_FILENAME.filename
      ))},
      NucleotideSequencePrefix{
         nucleotide_sequence_prefix.value_or(other.nucleotide_sequence_prefix.value_or(
            silo::preprocessing::DEFAULT_NUCLEOTIDE_SEQUENCE_PREFIX.prefix
         ))
      },
      UnalignedNucleotideSequencePrefix{unaligned_nucleotide_sequence_prefix.value_or(
         other.unaligned_nucleotide_sequence_prefix.value_or(
            silo::preprocessing::DEFAULT_UNALIGNED_NUC_SEQUENCE_PREFIX.prefix
         )
      )},
      GenePrefix{gene_prefix.value_or(
         other.gene_prefix.value_or(silo::preprocessing::DEFAULT_GENE_PREFIX.prefix)
      )}
   );
}

}  // namespace silo::preprocessing