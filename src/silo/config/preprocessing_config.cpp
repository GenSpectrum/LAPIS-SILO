#include "silo/config/preprocessing_config.h"

#include <filesystem>
#include <system_error>

#include <spdlog/spdlog.h>

#include "silo/common/fmt_formatters.h"
#include "silo/preprocessing/preprocessing_exception.h"

using silo::common::toDebugString;

namespace {
using silo::config::ConfigKeyPath;
using silo::config::YamlConfig;

ConfigKeyPath helpOptionKey() {
   return YamlConfig::stringToConfigKeyPath("help");
}
ConfigKeyPath preprocessingConfigOptionKey() {
   return YamlConfig::stringToConfigKeyPath("preprocessingConfig");
}
ConfigKeyPath defaultPreprocessingConfigOptionKey() {
   return YamlConfig::stringToConfigKeyPath("defaultPreprocessingConfig");
}
ConfigKeyPath inputDirectoryOptionKey() {
   return YamlConfig::stringToConfigKeyPath("inputDirectory");
}
ConfigKeyPath outputDirectoryOptionKey() {
   return YamlConfig::stringToConfigKeyPath("outputDirectory");
}
ConfigKeyPath intermediateResultsDirectoryOptionKey() {
   return YamlConfig::stringToConfigKeyPath("intermediateResultsDirectory");
}
ConfigKeyPath preprocessingDatabaseLocationOptionKey() {
   return YamlConfig::stringToConfigKeyPath("preprocessingDatabaseLocation");
}
ConfigKeyPath duckdbMemoryLimitInGTimeOptionKey() {
   return YamlConfig::stringToConfigKeyPath("duckdbMemoryLimitInG");
}
ConfigKeyPath lineageDefinitionsFilenameOptionKey() {
   return YamlConfig::stringToConfigKeyPath("lineageDefinitionsFilename");
}
ConfigKeyPath ndjsonInputFilenameOptionKey() {
   return YamlConfig::stringToConfigKeyPath("ndjsonInputFilename");
}
ConfigKeyPath databaseConfigFileOptionKey() {
   return YamlConfig::stringToConfigKeyPath("databaseConfig");
}
ConfigKeyPath referenceGenomeFilenameOptionKey() {
   return YamlConfig::stringToConfigKeyPath("referenceGenomeFilename");
}
}  // namespace

namespace silo::config {

// Specification of the fields in inputs to the PreprocessingConfig struct
ConfigSpecification PreprocessingConfig::getConfigSpecification() {
   return ConfigSpecification{
      .program_name = "silo preprocessing",
      .fields{
         ConfigValueSpecification::createWithoutDefault(
            helpOptionKey(), ConfigValueType::BOOL, "Show help text."
         ),
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
         "{} must be specified as preprocessing option.",
         ndjsonInputFilenameOptionKey().toDebugString()
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

bool PreprocessingConfig::asksForHelp() const {
   return help.has_value() && help.value();
}

void PreprocessingConfig::overwriteFrom(const VerifiedConfigSource& config_source) {
   if (auto var = config_source.getBool(helpOptionKey())) {
      help = var.value();
   }
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
   if(default_preprocessing_config.has_value()){
      result.emplace_back();
   }
   if(preprocessing_config.has_value()){
      result.emplace_back();
   }
   return result;
}

}  // namespace silo::config

#define CODE_FOR_FIELD(VARIABLE, FIELD_NAME) \
   fmt::format_to(ctx.out(), "{}: {}, ", #FIELD_NAME, toDebugString(VARIABLE.FIELD_NAME))

[[maybe_unused]] auto fmt::formatter<silo::config::PreprocessingConfig>::format(
   const silo::config::PreprocessingConfig& preprocessing_config,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   fmt::format_to(ctx.out(), "{{");
   CODE_FOR_FIELD(preprocessing_config, input_directory);
   CODE_FOR_FIELD(preprocessing_config, output_directory);
   CODE_FOR_FIELD(preprocessing_config, intermediate_results_directory);
   CODE_FOR_FIELD(preprocessing_config, preprocessing_database_location);
   CODE_FOR_FIELD(preprocessing_config, duckdb_memory_limit_in_g);
   CODE_FOR_FIELD(preprocessing_config, lineage_definitions_file);
   CODE_FOR_FIELD(preprocessing_config, ndjson_input_filename);
   CODE_FOR_FIELD(preprocessing_config, database_config_file);
   CODE_FOR_FIELD(preprocessing_config, reference_genome_file);
   return fmt::format_to(ctx.out(), "}}");
}

#undef CODE_FOR_FIELD
