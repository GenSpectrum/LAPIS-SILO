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
   if (ndjson_input_filename.has_value() && metadata_file.has_value()) {
      throw preprocessing::PreprocessingException(fmt::format(
         "Cannot specify both a ndjsonInputFilename ('{}') and metadataFilename('{}').",
         ndjson_input_filename.value().string(),
         metadata_file.value().string()
      ));
   }
   if (!ndjson_input_filename.has_value() && !metadata_file.has_value()) {
      throw preprocessing::PreprocessingException(fmt::format(
         "Neither a ndjsonInputFilename ('{}') nor a metadataFilename ('{}') was specified as "
         "preprocessing option.",
         NDJSON_INPUT_FILENAME_OPTION.toCamelCase(),
         METADATA_FILENAME_OPTION.toCamelCase()
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

[[nodiscard]] std::optional<uint32_t> PreprocessingConfig::getDuckdbMemoryLimitInG() const {
   return duckdb_memory_limit_in_g;
}

std::optional<std::filesystem::path> PreprocessingConfig::getLineageDefinitionsFilename() const {
   return lineage_definitions_file.has_value()
             ? std::optional(input_directory / *lineage_definitions_file)
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

std::filesystem::path PreprocessingConfig::getNucFilenameNoExtension(size_t sequence_idx) const {
   std::filesystem::path filename = input_directory;
   filename /= fmt::format("{}{}", nucleotide_sequence_prefix, sequence_idx);
   return filename;
}

std::filesystem::path PreprocessingConfig::getUnalignedNucFilenameNoExtension(size_t sequence_idx
) const {
   std::filesystem::path filename = input_directory;
   filename /= fmt::format("{}{}", unaligned_nucleotide_sequence_prefix, sequence_idx);
   return filename;
}

std::filesystem::path PreprocessingConfig::getGeneFilenameNoExtension(size_t sequence_idx) const {
   std::filesystem::path filename = input_directory;
   filename /= fmt::format("{}{}", gene_prefix, sequence_idx);
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
   if (auto value = config.getUInt32(DUCKDB_MEMORY_LIMIT_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         DUCKDB_MEMORY_LIMIT_OPTION.toString(),
         config.configType(),
         *value
      );
      duckdb_memory_limit_in_g = value;
   }
   if (auto value = config.getString(LINEAGE_DEFINITIONS_FILENAME_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         LINEAGE_DEFINITIONS_FILENAME_OPTION.toString(),
         config.configType(),
         *value
      );
      lineage_definitions_file = *value;
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
      "{{ input directory: '{}', lineage_definitions_file: {}, output_directory: '{}', "
      "metadata_file: {}, reference_genome_file: '{}',  gene_file_prefix: '{}',  "
      "nucleotide_sequence_file_prefix: '{}', unaligned_nucleotide_sequence_file_prefix: '{}', "
      "ndjson_filename: {}, preprocessing_database_location: {} }}",
      preprocessing_config.input_directory.string(),
      preprocessing_config.lineage_definitions_file.has_value()
         ? "'" + preprocessing_config.lineage_definitions_file->string() + "'"
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
