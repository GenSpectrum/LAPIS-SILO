#include "silo/config/util/yaml_config.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

using silo::config::YamlConfig;

YamlConfig::YamlConfig(const std::filesystem::path& filename)
    : filename(filename) {
   SPDLOG_INFO("Reading config from {}", filename.string());
   try {
      node = YAML::LoadFile(filename.string());
   } catch (const YAML::Exception& e) {
      throw std::runtime_error(
         fmt::format("Failed to read preprocessing config from {}: {}", filename.string(), e.what())
      );
   }
}

std::string YamlConfig::configType() const {
   return fmt::format("config file '{}'", filename.string());
}

bool YamlConfig::hasProperty(const std::string& key) const {
   return node[key].IsDefined();
}

std::string YamlConfig::getString(const std::string& key) const {
   return node[key].as<std::string>();
}

int32_t YamlConfig::getInt32(const std::string& key) const {
   return node[key].as<std::int32_t>();
}

uint32_t YamlConfig::getUInt32(const std::string& key) const {
   return node[key].as<std::uint32_t>();
}
