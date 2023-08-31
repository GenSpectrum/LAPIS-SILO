#include "silo_api/runtime_config.h"

#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

namespace YAML {

template <>
struct convert<silo_api::RuntimeConfig> {
   static bool decode(const Node& node, silo_api::RuntimeConfig& config) {
      config = silo_api::RuntimeConfig{
         node["dataDirectory"]
            ? std::optional<std::filesystem::path>(node["dataDirectory"].as<std::string>())
            : std::nullopt};

      return true;
   }
};

}  // namespace YAML

namespace silo_api {

RuntimeConfig RuntimeConfig::readFromFile(const std::filesystem::path& config_path) {
   SPDLOG_INFO("Reading runtime config from {}", config_path.string());

   try {
      return YAML::LoadFile(config_path.string()).as<RuntimeConfig>();
   } catch (const YAML::Exception& e) {
      throw std::runtime_error(
         "Failed to read runtime config from " + config_path.string() + ": " + std::string(e.what())
      );
   }
}

}  // namespace silo_api
