#pragma once

#include <filesystem>
#include <utility>

#include <yaml-cpp/yaml.h>

#include "config/config_source_interface.h"

class YamlFileContents;

class YamlFile : public ConfigSource {
  public:
   [[nodiscard]] std::string configContext() const override;
   [[nodiscard]] std::string configKeyPathToString(const ConfigKeyPath& config_key_path
   ) const override;

   static YamlFileContents readFile(const std::filesystem::path& path);
};

class YamlFileContents : public VerifyConfigSource {
   std::filesystem::path content_source;
   std::vector<std::pair<ConfigKeyPath, YAML::Node>> paths;

   friend class VerifiedYamlFile;

  public:
   YamlFileContents(
      std::filesystem::path content_source,
      std::vector<std::pair<ConfigKeyPath, YAML::Node>> paths
   )
       : content_source(std::move(content_source)),
         paths(std::move(paths)) {}

   [[nodiscard]] std::string configContext() const override;
   [[nodiscard]] std::string configKeyPathToString(const ConfigKeyPath& config_key_path
   ) const override;

   [[nodiscard]] std::unique_ptr<VerifiedConfigSource> verify(
      const std::span<const std::pair<ConfigKeyPath, const ConfigValue*>>& config_structs
   ) override;
};

class VerifiedYamlFile : public VerifiedConfigSource {
   YamlFileContents base;

  public:
   explicit VerifiedYamlFile(YamlFileContents&& base)
       : base(base) {}

   [[nodiscard]] std::string configContext() const override;
   [[nodiscard]] std::string configKeyPathToString(const ConfigKeyPath& config_key_path
   ) const override;

   [[nodiscard]] std::optional<std::string> getString(const ConfigKeyPath& config_key_path
   ) const override;
   [[nodiscard]] const std::vector<std::string>* positionalArgs() const override;
};
