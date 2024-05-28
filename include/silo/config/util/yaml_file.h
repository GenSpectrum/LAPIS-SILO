#pragma once

#include <filesystem>

#include <yaml-cpp/yaml.h>

#include "abstract_config_source.h"

namespace silo::config {

class YamlFile : public silo::config::AbstractConfigSource {
   std::filesystem::path filename;
   YAML::Node node;

  public:
   explicit YamlFile(const std::filesystem::path& filename);

   std::string configType() const override;

   bool hasProperty(const Option& option) const override;

   std::optional<std::string> getString(const Option& option) const override;
};

}  // namespace silo::config
