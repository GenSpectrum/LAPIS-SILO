#pragma once

#include <filesystem>

#include <yaml-cpp/yaml.h>

#include "abstract_config.h"

namespace silo::config {

class YamlConfig : public silo::config::AbstractConfig {
   std::filesystem::path filename;
   YAML::Node node;

  public:
   YamlConfig(const std::filesystem::path& filename);

   std::string configType() const override;

   bool hasProperty(const std::string& key) const override;

   std::string getString(const std::string& key) const override;

   int32_t getInt32(const std::string& key) const override;

   uint32_t getUInt32(const std::string& key) const override;
};

}  // namespace silo::config
