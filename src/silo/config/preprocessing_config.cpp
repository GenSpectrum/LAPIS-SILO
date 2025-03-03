#include "silo/config/preprocessing_config.h"

#include <filesystem>
#include <system_error>

#include <spdlog/spdlog.h>

#include "config/config_interface.h"
#include "silo/common/fmt_formatters.h"
#include "silo/common/json_type_definitions.h"
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
ConfigKeyPath lineageDefinitionsFilenameOptionKey() {
   return YamlFile::stringToConfigKeyPath("lineageDefinitionsFilename");
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
      .attribute_specifications{
         ConfigAttributeSpecification::createWithoutDefault(
            preprocessingConfigOptionKey(),
            ConfigValueType::PATH,
            "The path to a preprocessing config that should be read before overwriting\n"
            "its values with environment variables and other CLI arguments."
         ),
         ConfigAttributeSpecification::createWithoutDefault(
            defaultPreprocessingConfigOptionKey(),
            ConfigValueType::PATH,
            "The path to a default preprocessing config that should be read first.\n"
            "This path will often be set by an environment variable, thus \n"
            "providing defaults to a silo in a specific environment (e.g. Docker)."
         ),
         ConfigAttributeSpecification::createWithDefault(
            inputDirectoryOptionKey(),
            ConfigValue::fromPath("./"),
            "The path to the directory with the input files."
         ),
         ConfigAttributeSpecification::createWithDefault(
            outputDirectoryOptionKey(),
            ConfigValue::fromPath(DEFAULT_OUTPUT_DIRECTORY),
            "The path to the directory to hold the output files."
         ),
         ConfigAttributeSpecification::createWithoutDefault(
            lineageDefinitionsFilenameOptionKey(),
            ConfigValueType::PATH,
            "File name of the file holding the lineage definitions. Relative from inputDirectory."
         ),
         ConfigAttributeSpecification::createWithDefault(
            databaseConfigFileOptionKey(),
            ConfigValue::fromPath("database_config.yaml"),
            "File name of the file holding the database table configuration. Relative from "
            "inputDirectory."
         ),
         ConfigAttributeSpecification::createWithDefault(
            referenceGenomeFilenameOptionKey(),
            ConfigValue::fromPath("reference_genomes.json"),
            "File name of the file holding the reference genome. Relative from inputDirectory."
         ),
      }
   };
}

PreprocessingConfig PreprocessingConfig::withDefaults() {
   PreprocessingConfig result;
   result.overwriteFrom(getConfigSpecification().getConfigSourceFromDefaults());
   return result;
}

void PreprocessingConfig::validate() const {}

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

void PreprocessingConfig::overwriteFrom(const VerifiedConfigAttributes& config_source) {
   if (auto var = config_source.getPath(inputDirectoryOptionKey())) {
      input_directory = var.value();
   }
   if (auto var = config_source.getPath(lineageDefinitionsFilenameOptionKey())) {
      lineage_definitions_file = var.value();
   }
   if (auto var = config_source.getPath(databaseConfigFileOptionKey())) {
      database_config_file = var.value();
   }
   if (auto var = config_source.getPath(referenceGenomeFilenameOptionKey())) {
      reference_genome_file = var.value();
   }
   if (auto var = config_source.getPath(outputDirectoryOptionKey())) {
      output_directory = var.value();
   }
}

std::vector<std::filesystem::path> PreprocessingConfig::getConfigFilePaths(
   const VerifiedCommandLineArguments& cmd_source,
   const VerifiedConfigAttributes& env_source
) {
   std::vector<std::filesystem::path> result;
   auto default_runtime_config =
      getConfigFilePath(defaultPreprocessingConfigOptionKey(), cmd_source, env_source);
   if (default_runtime_config.has_value()) {
      result.emplace_back(default_runtime_config.value());
   }
   auto runtime_config = getConfigFilePath(preprocessingConfigOptionKey(), cmd_source, env_source);
   if (runtime_config.has_value()) {
      result.emplace_back(runtime_config.value());
   }
   return result;
}

}  // namespace silo::config

[[maybe_unused]] auto fmt::formatter<silo::config::PreprocessingConfig>::format(
   const silo::config::PreprocessingConfig& preprocessing_config,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   nlohmann::json json = preprocessing_config;
   return fmt::format_to(ctx.out(), "{}", json);
}
