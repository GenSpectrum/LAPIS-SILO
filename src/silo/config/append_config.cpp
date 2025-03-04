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
ConfigKeyPath databaseStateConfigOptionKey() {
   return YamlFile::stringToConfigKeyPath("databaseState");
}
ConfigKeyPath appendFile() {
   return YamlFile::stringToConfigKeyPath("appendFile");
}
ConfigKeyPath outputFolder() {
   return YamlFile::stringToConfigKeyPath("outputFolder");
}
}  // namespace

namespace silo::config {

void AppendConfig::validate() const {
   if (!std::filesystem::exists(database_state)) {
      throw silo::config::ConfigException(
         "The folder '" + database_state.string() + "' does not exist"
      );
   }
   if (!std::filesystem::exists(append_file)) {
      throw silo::config::ConfigException("The file '" + append_file.string() + "' does not exist");
   }
   if (!std::filesystem::exists(output_folder)) {
      throw silo::config::ConfigException(
         "The folder '" + output_folder.string() + "' does not exist"
      );
   }
}

AppendConfig AppendConfig::withDefaults() {
   AppendConfig result;
   result.overwriteFrom(getConfigSpecification().getConfigSourceFromDefaults());
   return result;
}

ConfigSpecification AppendConfig::getConfigSpecification() {
   return ConfigSpecification{
      .program_name = "silo append",
      .attribute_specifications{
         ConfigAttributeSpecification::createWithoutDefault(
            databaseStateConfigOptionKey(),
            ConfigValueType::PATH,
            "The path to a directory that contains the database state that should be appended to."
         ),
         ConfigAttributeSpecification::createWithoutDefault(
            appendFile(),
            ConfigValueType::PATH,
            "The path to a file that contains the data that should be appended to the database."
         ),
         ConfigAttributeSpecification::createWithDefault(
            outputFolder(), ConfigValue::fromPath("."), "The path to a folder which should contain."
         )
      }
   };
}

void AppendConfig::overwriteFrom(const VerifiedConfigAttributes& config_source) {
   if (auto var = config_source.getPath(databaseStateConfigOptionKey())) {
      database_state = var.value();
   }
   if (auto var = config_source.getPath(appendFile())) {
      append_file = var.value();
   }
   if (auto var = config_source.getPath(outputFolder())) {
      output_folder = var.value();
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
