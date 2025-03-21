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
ConfigKeyPath ndjsonInputFilenameOptionKey() {
   return YamlFile::stringToConfigKeyPath("ndjsonInputFilename");
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
         ConfigAttributeSpecification::createWithDefault(
            ndjsonInputFilenameOptionKey(),
            ConfigValue::fromPath("reference_genomes.json"),
            "File name of the file holding the reference genome. Relative from inputDirectory."
         ),
      }
   };
}

PreprocessingConfig PreprocessingConfig::withDefaults() {
   PreprocessingConfig result;
   result.initialize_config.overwriteFrom(
      InitializeConfig::getConfigSpecification().getConfigSourceFromDefaults()
   );
   result.overwriteFrom(getConfigSpecification().getConfigSourceFromDefaults());
   return result;
}

void PreprocessingConfig::validate() const {
   if (!input_file.has_value()) {
      throw silo::preprocessing::PreprocessingException("The value 'inputFile' must be set.");
   }
}

void PreprocessingConfig::overwriteFrom(const VerifiedConfigAttributes& config_source) {
   if (auto var = config_source.getPath(inputDirectoryOptionKey())) {
      initialize_config.input_directory = var.value();
   }
   if (auto var = config_source.getPath(lineageDefinitionsFilenameOptionKey())) {
      initialize_config.lineage_definitions_file = var.value();
   }
   if (auto var = config_source.getPath(databaseConfigFileOptionKey())) {
      initialize_config.database_config_file = var.value();
   }
   if (auto var = config_source.getPath(referenceGenomeFilenameOptionKey())) {
      initialize_config.reference_genome_file = var.value();
   }
   if (auto var = config_source.getPath(outputDirectoryOptionKey())) {
      initialize_config.output_directory = var.value();
   }
   if (auto var = config_source.getPath(ndjsonInputFilenameOptionKey())) {
      input_file = var.value();
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
