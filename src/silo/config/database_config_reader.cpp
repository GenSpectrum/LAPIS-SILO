#include "silo/config/database_config_reader.h"

#include <iostream>

#include <yaml-cpp/yaml.h>

namespace YAML {
template <>
struct convert<silo::DatabaseConfig> {
   static bool decode(const Node& node, silo::DatabaseConfig& config) {
      config.schema = node["schema"].as<silo::DatabaseSchema>();
      return true;
   }
};

template <>
struct convert<silo::DatabaseSchema> {
   static bool decode(const Node& node, silo::DatabaseSchema& schema) {
      schema.instance_name = node["instanceName"].as<std::string>();
      schema.primary_key = node["primaryKey"].as<std::string>();
      for (const auto& metadata : node["metadata"]) {
         schema.metadata.push_back(metadata.as<silo::DatabaseMetadata>());
      }
      return true;
   }
};

template <>
struct convert<silo::DatabaseMetadata> {
   static bool decode(const Node& node, silo::DatabaseMetadata& metadata) {
      metadata.name = node["name"].as<std::string>();
      metadata.type = node["type"].as<std::string>();
      return true;
   }
};

}  // namespace YAML

namespace silo {
DatabaseConfig DatabaseConfigReader::readConfig(const std::filesystem::path& config_path) {
   return YAML::LoadFile(config_path.string()).as<DatabaseConfig>();
}
}  // namespace silo
