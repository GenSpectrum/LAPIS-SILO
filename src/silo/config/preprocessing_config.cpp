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
   if (!ndjson_input_filename.has_value()) {
      throw preprocessing::PreprocessingException(fmt::format(
         "{} must be specified as preprocessing option.", NDJSON_INPUT_FILENAME_OPTION.toCamelCase()
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

std::optional<std::filesystem::path> PreprocessingConfig::getNdjsonInputFilename() const {
   return ndjson_input_filename.has_value()
             ? std::optional(input_directory / *ndjson_input_filename)
             : std::nullopt;
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
   if (auto value = config.getString(REFERENCE_GENOME_FILENAME_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         REFERENCE_GENOME_FILENAME_OPTION.toString(),
         config.configType(),
         *value
      );
      reference_genome_file = *value;
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
      "reference_genome_file: '{}', ndjson_filename: {}, preprocessing_database_location: {} }}",
      preprocessing_config.input_directory.string(),
      preprocessing_config.lineage_definitions_file.has_value()
         ? "'" + preprocessing_config.lineage_definitions_file->string() + "'"
         : "none",
      preprocessing_config.output_directory.string(),
      preprocessing_config.reference_genome_file.string(),
      preprocessing_config.ndjson_input_filename.has_value()
         ? "'" + preprocessing_config.ndjson_input_filename->string() + "'"
         : "none",
      preprocessing_config.preprocessing_database_location.has_value()
         ? "'" + preprocessing_config.preprocessing_database_location->string() + "'"
         : "none"
   );
}
