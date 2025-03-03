#include "silo/config/append_config.h"

#include "config/config_interface.h"
#include "config/source/yaml_file.h"
#include "silo/common/json_type_definitions.h"
#include "silo/config/config_defaults.h"

using silo::config::ConfigKeyPath;
using silo::config::YamlFile;

namespace {
ConfigKeyPath appendConfigOptionKey() {
   return YamlFile::stringToConfigKeyPath("appendConfig");
}
ConfigKeyPath siloDirectoryOptionKey() {
   return YamlFile::stringToConfigKeyPath("siloDirectory");
}
ConfigKeyPath appendFileOptionKey() {
   return YamlFile::stringToConfigKeyPath("appendFile");
}
ConfigKeyPath siloDataSourceOptionKey() {
   return YamlFile::stringToConfigKeyPath("siloDataSource");
}
ConfigKeyPath dataVersionOptionKey() {
   return YamlFile::stringToConfigKeyPath("dataVersion");
}
}  // namespace

namespace silo::config {

void AppendConfig::validate() const {}

AppendConfig AppendConfig::withDefaults() {
   AppendConfig result;
   result.overwriteFrom(getConfigSpecification().getConfigSourceFromDefaults());
   return result;
}

ConfigSpecification AppendConfig::getConfigSpecification() {
   return ConfigSpecification{
      .program_name = "silo append",
      .attribute_specifications{
         ConfigAttributeSpecification::createWithDefault(
            siloDirectoryOptionKey(),
            ConfigValue::fromPath("."),
            "The path to a silo-directory, a directory that contains silo outputs."
         ),
         ConfigAttributeSpecification::createWithoutDefault(
            dataVersionOptionKey(), ConfigValueType::STRING, "The data version in the silo folder that should be appended to. If no data version is given, it will automatically append to the most recent data version instead."
         ),
         ConfigAttributeSpecification::createWithoutDefault(
            appendFileOptionKey(),
            ConfigValueType::PATH,
            "The path to a file that contains the data that should be appended to the database. If no file is given, the data is expected on stdin instead."
         ),
         ConfigAttributeSpecification::createWithoutDefault(
            siloDataSourceOptionKey(),
            ConfigValueType::PATH,
            "A directory that contains a valid silo state. If this is not given, the most recent database state from the silo-directory is taken instead."
         )
      }
   };
}

void AppendConfig::overwriteFrom(const VerifiedConfigAttributes& config_source) {
   if (auto var = config_source.getPath(siloDirectoryOptionKey())) {
      silo_directory = var.value();
   }
   if (auto var = config_source.getString(dataVersionOptionKey())) {
      data_version = var.value();
   }
   if (auto var = config_source.getPath(appendFileOptionKey())) {
      append_file = var.value();
   }
   if (auto var = config_source.getPath(siloDataSourceOptionKey())) {
      silo_data_source = var.value();
   }
}

std::vector<std::filesystem::path> AppendConfig::getConfigFilePaths(
   const VerifiedCommandLineArguments& cmd_source,
   const VerifiedConfigAttributes& env_source
) {
   std::vector<std::filesystem::path> result;
   auto append_config = getConfigFilePath(appendConfigOptionKey(), cmd_source, env_source);
   if (append_config.has_value()) {
      result.emplace_back(append_config.value());
   }
   return result;
}

}  // namespace silo::config

[[maybe_unused]] auto fmt::formatter<silo::config::AppendConfig>::format(
   const silo::config::AppendConfig& append_config,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   nlohmann::json json = append_config;
   return fmt::format_to(ctx.out(), "{}", json);
}
