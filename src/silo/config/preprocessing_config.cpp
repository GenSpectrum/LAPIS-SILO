#include "silo/config/preprocessing_config.h"

#include <filesystem>
#include <system_error>

#include <spdlog/spdlog.h>

#include "silo/config/util/abstract_config_source.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::config {

void PreprocessingConfig::validate() const {
   if (!std::filesystem::exists(input_directory)) {
      throw preprocessing::PreprocessingException(input_directory.string() + " does not exist");
   }
   if (ndjson_input_filename.has_value() && metadata_file) {
      throw preprocessing::PreprocessingException(fmt::format(
         "Cannot specify both a ndjsonInputFilename ('{}') and metadataFilename('{}').",
         ndjson_input_filename.value().string(),
         metadata_file.value().string()
      ));
   }
}

std::filesystem::path PreprocessingConfig::getOutputDirectory() const {
   return output_directory;
}

std::filesystem::path PreprocessingConfig::getIntermediateResultsDirectory() const {
   return intermediate_results_directory;
}

std::optional<std::filesystem::path> PreprocessingConfig::getPreprocessingDatabaseLocation() const {
   return preprocessing_database_location;
}

std::optional<std::filesystem::path> PreprocessingConfig::getPangoLineageDefinitionFilename(
) const {
   return pango_lineage_definition_file.has_value()
             ? std::optional(input_directory / *pango_lineage_definition_file)
             : std::nullopt;
}

std::filesystem::path PreprocessingConfig::getReferenceGenomeFilename() const {
   return input_directory / reference_genome_file;
}

std::optional<std::filesystem::path> PreprocessingConfig::getMetadataInputFilename() const {
   return metadata_file.has_value() ? std::optional(input_directory / *metadata_file)
                                    : std::nullopt;
}

std::optional<std::filesystem::path> PreprocessingConfig::getNdjsonInputFilename() const {
   return ndjson_input_filename.has_value()
             ? std::optional(input_directory / *ndjson_input_filename)
             : std::nullopt;
}

std::filesystem::path PreprocessingConfig::getNucFilenameNoExtension(std::string_view nuc_name
) const {
   std::filesystem::path filename = input_directory;
   filename /= fmt::format("{}{}", nucleotide_sequence_prefix, nuc_name);
   return filename;
}

std::filesystem::path PreprocessingConfig::getUnalignedNucFilenameNoExtension(
   std::string_view nuc_name
) const {
   std::filesystem::path filename = input_directory;
   filename /= fmt::format("{}{}", unaligned_nucleotide_sequence_prefix, nuc_name);
   return filename;
}

std::filesystem::path PreprocessingConfig::getGeneFilenameNoExtension(std::string_view gene_name
) const {
   std::filesystem::path filename = input_directory;
   filename /= fmt::format("{}{}", gene_prefix, gene_name);
   return filename;
}

std::filesystem::path PreprocessingConfig::getNucleotideInsertionsFilename() const {
   return input_directory / nuc_insertions_filename;
}

std::filesystem::path PreprocessingConfig::getAminoAcidInsertionsFilename() const {
   return input_directory / aa_insertions_filename;
}

void PreprocessingConfig::overwrite(const silo::config::AbstractConfigSource& config) {
   if (auto value = config.getString(INPUT_DIRECTORY_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         INPUT_DIRECTORY_OPTION.toString(),
         config.configType(),
         *value
      );
      input_directory = *value;
   }
   if (auto value = config.getString(OUTPUT_DIRECTORY_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         OUTPUT_DIRECTORY_OPTION.toString(),
         config.configType(),
         *value
      );
      output_directory = *value;
   }
   if (auto value = config.getString(INTERMEDIATE_RESULTS_DIRECTORY_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         INTERMEDIATE_RESULTS_DIRECTORY_OPTION.toString(),
         config.configType(),
         *value
      );
      intermediate_results_directory = *value;
   }
   if (auto value = config.getString(PREPROCESSING_DATABASE_LOCATION_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         PREPROCESSING_DATABASE_LOCATION_OPTION.toString(),
         config.configType(),
         *value
      );
      preprocessing_database_location = *value;
   }
   if (auto value = config.getString(PANGO_LINEAGE_DEFINITION_FILENAME_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         PANGO_LINEAGE_DEFINITION_FILENAME_OPTION.toString(),
         config.configType(),
         *value
      );
      pango_lineage_definition_file = *value;
   }
   if (auto value = config.getString(NDJSON_INPUT_FILENAME_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         NDJSON_INPUT_FILENAME_OPTION.toString(),
         config.configType(),
         *value
      );
      ndjson_input_filename = *value;
   }
   if (auto value = config.getString(METADATA_FILENAME_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         METADATA_FILENAME_OPTION.toString(),
         config.configType(),
         *value
      );
      metadata_file = *value;
   }
   if (auto value = config.getString(REFERENCE_GENOME_FILENAME_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         REFERENCE_GENOME_FILENAME_OPTION.toString(),
         config.configType(),
         *value
      );
      reference_genome_file = *value;
   }
   if (auto value = config.getString(NUCLEOTIDE_SEQUENCE_PREFIX_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         NUCLEOTIDE_SEQUENCE_PREFIX_OPTION.toString(),
         config.configType(),
         *value
      );
      nucleotide_sequence_prefix = *value;
   }
   if (auto value = config.getString(UNALIGNED_NUCLEOTIDE_SEQUENCE_PREFIX_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         UNALIGNED_NUCLEOTIDE_SEQUENCE_PREFIX_OPTION.toString(),
         config.configType(),
         *value
      );
      unaligned_nucleotide_sequence_prefix = *value;
   }
   if (auto value = config.getString(GENE_PREFIX_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}", GENE_PREFIX_OPTION.toString(), config.configType(), *value
      );
      gene_prefix = *value;
   }
   if (auto value = config.getString(NUCLEOTIDE_INSERTIONS_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         NUCLEOTIDE_INSERTIONS_OPTION.toString(),
         config.configType(),
         *value
      );
      nuc_insertions_filename = *value;
   }
   if (auto value = config.getString(AMINO_ACID_INSERTIONS_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         AMINO_ACID_INSERTIONS_OPTION.toString(),
         config.configType(),
         *value
      );
      aa_insertions_filename = *value;
   }
}

}  // namespace silo::config

[[maybe_unused]] auto fmt::formatter<silo::config::PreprocessingConfig>::format(
   const silo::config::PreprocessingConfig& preprocessing_config,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   return fmt::format_to(
      ctx.out(),
      "{{ input directory: '{}', pango_lineage_definition_file: {}, output_directory: '{}', "
      "metadata_file: '{}', reference_genome_file: '{}',  gene_file_prefix: '{}',  "
      "nucleotide_sequence_file_prefix: '{}', unalgined_nucleotide_sequence_file_prefix: '{}', "
      "ndjson_filename: {}, "
      "preprocessing_database_location: {} }}",
      preprocessing_config.input_directory.string(),
      preprocessing_config.pango_lineage_definition_file.has_value()
         ? "'" + preprocessing_config.pango_lineage_definition_file->string() + "'"
         : "none",
      preprocessing_config.output_directory.string(),
      preprocessing_config.metadata_file.has_value()
         ? "'" + preprocessing_config.metadata_file->string() + "'"
         : "none",
      preprocessing_config.reference_genome_file.string(),
      preprocessing_config.gene_prefix,
      preprocessing_config.nucleotide_sequence_prefix,
      preprocessing_config.unaligned_nucleotide_sequence_prefix,
      preprocessing_config.ndjson_input_filename.has_value()
         ? "'" + preprocessing_config.ndjson_input_filename->string() + "'"
         : "none",
      preprocessing_config.preprocessing_database_location.has_value()
         ? "'" + preprocessing_config.preprocessing_database_location->string() + "'"
         : "none"
   );
}
