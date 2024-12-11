#pragma once

#include <filesystem>
#include <utility>

#include <yaml-cpp/yaml.h>

#include "config/config_source_interface.h"

namespace silo::config {

class YamlFile {
   std::string debug_context;
   std::unordered_map<ConfigKeyPath, YAML::Node> yaml_fields;

   YamlFile(std::string debug_context, std::unordered_map<ConfigKeyPath, YAML::Node> yaml_fields)
       : debug_context(std::move(debug_context)),
         yaml_fields(std::move(yaml_fields)) {}

  public:
   std::string debugContext() const;

   // Make yaml_fields accessible for testing
   const std::unordered_map<ConfigKeyPath, YAML::Node>& getYamlFields() const;

   static YamlFile readFile(const std::filesystem::path& path);

   static YamlFile fromYAML(const std::string& debug_context, const std::string& yaml_string);

   static std::string configKeyPathToString(const ConfigKeyPath& key_path);

   static ConfigKeyPath stringToConfigKeyPath(const std::string& key_path_string);

   using VerifiedType = VerifiedConfigAttributes;
   [[nodiscard]] VerifiedType verify(const ConfigSpecification& config_specification) const;
};
static_assert(ConfigSource<YamlFile>, "YamlFile does not satisfy ConfigSource");
}  // namespace silo::config
