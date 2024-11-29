#include "silo/config/preprocessing_config.h"

#include <filesystem>
#include <system_error>

#include <spdlog/spdlog.h>

#include "silo/common/fmt_formatters.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace {
using silo::config::ConfigKeyPath;
using silo::config::YamlFile;

// Using functions instead of global variables because of
// initialization order issues.

ConfigKeyPath preprocessingConfigOptionKey() {
   return YamlFile::stringToConfigKeyPath("preprocessingConfig");
}
ConfigKeyPath defaultPreprocessingConfigOptionKey() {
   return YamlFile::stringToConfigKeyPath("defaultPreprocessingConfig");
}
ConfigKeyPath inputDirectoryOptionKey() {
   return YamlFile::stringToConfigKeyPath("inputDirectory");
}
ConfigKeyPath outputDirectoryOptionKey() {
   return YamlFile::stringToConfigKeyPath("outputDirectory");
}
ConfigKeyPath intermediateResultsDirectoryOptionKey() {
   return YamlFile::stringToConfigKeyPath("intermediateResultsDirectory");
}
ConfigKeyPath preprocessingDatabaseLocationOptionKey() {
   return YamlFile::stringToConfigKeyPath("preprocessingDatabaseLocation");
}
ConfigKeyPath duckdbMemoryLimitInGTimeOptionKey() {
   return YamlFile::stringToConfigKeyPath("duckdbMemoryLimitInG");
}
ConfigKeyPath lineageDefinitionsFilenameOptionKey() {
   return YamlFile::stringToConfigKeyPath("lineageDefinitionsFilename");
}
ConfigKeyPath ndjsonInputFilenameOptionKey() {
   return YamlFile::stringToConfigKeyPath("ndjsonInputFilename");
}
ConfigKeyPath databaseConfigFileOptionKey() {
   return YamlFile::stringToConfigKeyPath("databaseConfig");
}
ConfigKeyPath referenceGenomeFilenameOptionKey() {
   return YamlFile::stringToConfigKeyPath("referenceGenomeFilename");
}
}  // namespace

namespace silo::config {

// Specification of the fields in inputs to the PreprocessingConfig struct
ConfigSpecification PreprocessingConfig::getConfigSpecification() {
   return ConfigSpecification{
      .program_name = "silo preprocessing",
      .fields{
         ConfigValueSpecification::createWithoutDefault(
            preprocessingConfigOptionKey(),
            ConfigValueType::PATH,
            "Path to a preprocessing config that should be read before overwriting its values "
            "with environment variables and other CLI arguments."
         ),
         ConfigValueSpecification::createWithoutDefault(
            defaultPreprocessingConfigOptionKey(),
            ConfigValueType::PATH,
            "Path to a default preprocessing config that should be read first."
            "This value will often be set as an environment variable, "
            "in cases where defaults are provided to SILO."
         ),
         ConfigValueSpecification::createWithDefault(
            inputDirectoryOptionKey(),
            ConfigValue::fromPath("./"),
            "the path to the directory with the input files"
         ),
         ConfigValueSpecification::createWithDefault(
            outputDirectoryOptionKey(),
            ConfigValue::fromPath(DEFAULT_OUTPUT_DIRECTORY),
            "the path to the directory to hold the output files"
         ),
         ConfigValueSpecification::createWithDefault(
            intermediateResultsDirectoryOptionKey(),
            ConfigValue::fromPath("./temp/"),
            "the path to the directory to hold temporary files"
         ),
         ConfigValueSpecification::createWithoutDefault(
            preprocessingDatabaseLocationOptionKey(),
            ConfigValueType::PATH,
            "the file where the duckdb database will be stored, which is used during preprocessing"
         ),
         ConfigValueSpecification::createWithoutDefault(
            duckdbMemoryLimitInGTimeOptionKey(),
            ConfigValueType::UINT32,
            "DuckDB memory limit in GB"
         ),
         ConfigValueSpecification::createWithoutDefault(
            lineageDefinitionsFilenameOptionKey(),
            ConfigValueType::PATH,
            "file name of the file holding the lineage definitions"
         ),
         ConfigValueSpecification::createWithoutDefault(
            ndjsonInputFilenameOptionKey(),
            ConfigValueType::PATH,
            "file name of the file holding NDJSON input"
         ),
         ConfigValueSpecification::createWithDefault(
            databaseConfigFileOptionKey(),
            ConfigValue::fromPath("database_config.yaml"),
            "file name of the file holding the database table configuration"
         ),
         ConfigValueSpecification::createWithDefault(
            referenceGenomeFilenameOptionKey(),
            ConfigValue::fromPath("reference_genomes.json"),
            "file name of the file holding the reference genome"
         ),
      }
   };
}

PreprocessingConfig::PreprocessingConfig() {
   overwriteFrom(getConfigSpecification().getConfigSourceFromDefaults());
}

void PreprocessingConfig::validate() const {
   if (!std::filesystem::exists(input_directory)) {
      throw preprocessing::PreprocessingException(input_directory.string() + " does not exist");
   }
   if (!ndjson_input_filename.has_value()) {
      throw preprocessing::PreprocessingException(fmt::format(
         "'{}' must be specified as preprocessing option.",
         YamlFile::configKeyPathToString(ndjsonInputFilenameOptionKey())
      ));
   }
}

std::optional<uint32_t> PreprocessingConfig::getDuckdbMemoryLimitInG() const {
   return duckdb_memory_limit_in_g;
}

std::filesystem::path PreprocessingConfig::getDatabaseConfigFilename() const {
   return input_directory / database_config_file;
}

std::optional<std::filesystem::path> PreprocessingConfig::getLineageDefinitionsFilename() const {
   return lineage_definitions_file.has_value()
             ? std::optional(input_directory / lineage_definitions_file.value())
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

void PreprocessingConfig::overwriteFrom(const VerifiedConfigAttributes& config_source) {
   if (auto var = config_source.getPath(preprocessingConfigOptionKey())) {
      preprocessing_config = var.value();
   }
   if (auto var = config_source.getPath(defaultPreprocessingConfigOptionKey())) {
      default_preprocessing_config = var.value();
   }
   if (auto var = config_source.getPath(inputDirectoryOptionKey())) {
      input_directory = var.value();
   }
   if (auto var = config_source.getPath(outputDirectoryOptionKey())) {
      output_directory = var.value();
   }
   if (auto var = config_source.getPath(intermediateResultsDirectoryOptionKey())) {
      intermediate_results_directory = var.value();
   }
   if (auto var = config_source.getPath(preprocessingDatabaseLocationOptionKey())) {
      preprocessing_database_location = var.value();
   }
   if (auto var = config_source.getUint32(duckdbMemoryLimitInGTimeOptionKey())) {
      duckdb_memory_limit_in_g = var.value();
   }
   if (auto var = config_source.getPath(lineageDefinitionsFilenameOptionKey())) {
      lineage_definitions_file = var.value();
   }
   if (auto var = config_source.getPath(ndjsonInputFilenameOptionKey())) {
      ndjson_input_filename = var.value();
   }
   if (auto var = config_source.getPath(databaseConfigFileOptionKey())) {
      database_config_file = var.value();
   }
   if (auto var = config_source.getPath(referenceGenomeFilenameOptionKey())) {
      reference_genome_file = var.value();
   }
}

std::vector<std::filesystem::path> PreprocessingConfig::getConfigPaths() const {
   std::vector<std::filesystem::path> result;
   if (default_preprocessing_config.has_value()) {
      result.emplace_back();
   }
   if (preprocessing_config.has_value()) {
      result.emplace_back();
   }
   return result;
}

}  // namespace silo::config

namespace silo::common {
// Using a macro to get the declared variable name #FIELD_NAME
#define ADD_FIELD_TO_RESULT(VARIABLE, FIELD_NAME) \
   result += fmt::format("{}: {}, ", #FIELD_NAME, toDebugString(VARIABLE.FIELD_NAME))

std::string toDebugString(const silo::config::PreprocessingConfig& preprocessing_config) {
   std::string result = "{";
   ADD_FIELD_TO_RESULT(preprocessing_config, input_directory);
   ADD_FIELD_TO_RESULT(preprocessing_config, output_directory);
   ADD_FIELD_TO_RESULT(preprocessing_config, intermediate_results_directory);
   ADD_FIELD_TO_RESULT(preprocessing_config, preprocessing_database_location);
   ADD_FIELD_TO_RESULT(preprocessing_config, duckdb_memory_limit_in_g);
   ADD_FIELD_TO_RESULT(preprocessing_config, lineage_definitions_file);
   ADD_FIELD_TO_RESULT(preprocessing_config, ndjson_input_filename);
   ADD_FIELD_TO_RESULT(preprocessing_config, database_config_file);
   ADD_FIELD_TO_RESULT(preprocessing_config, reference_genome_file);
   result += "}";
   return result;
}
#undef ADD_FIELD_TO_RESULT
}  // namespace silo::common

[[maybe_unused]] auto fmt::formatter<silo::config::PreprocessingConfig>::format(
   const silo::config::PreprocessingConfig& preprocessing_config,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   return fmt::format_to(ctx.out(), "{}", silo::common::toDebugString(preprocessing_config));
}
