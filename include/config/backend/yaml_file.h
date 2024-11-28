#pragma once

#include <filesystem>
#include <utility>

#include <yaml-cpp/yaml.h>

#include "config/config_source.h"

namespace silo::config {

class YamlConfig : public ConfigBackend {
   std::string error_context;
   std::unordered_map<ConfigKeyPath, YAML::Node> yaml_fields;

   YamlConfig(std::string error_context, std::unordered_map<ConfigKeyPath, YAML::Node> yaml_fields)
       : error_context(std::move(error_context)),
         yaml_fields(std::move(yaml_fields)) {}

   std::string errorContext() const;

  public:
   [[nodiscard]] VerifiedConfigSource verify(const ConfigSpecification& config_specification
   ) const override;

   const std::unordered_map<ConfigKeyPath, YAML::Node>& getYamlFields() const;

   static YamlConfig readFile(const std::filesystem::path& path);

   static YamlConfig fromYAML(const std::string& error_context, const std::string& yaml_string);

   static std::string configKeyPathToString(const ConfigKeyPath& key_path);

   static ConfigKeyPath stringToConfigKeyPath(const std::string& key_path_string);
};

}  // namespace silo::config
