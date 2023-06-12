#include "silo/config/database_config_reader.h"
#include "silo/config/config_exception.h"

#include <iostream>

#include <yaml-cpp/yaml.h>

namespace YAML {
template <>
struct convert<silo::config::DatabaseConfig> {
   static bool decode(const Node& node, silo::config::DatabaseConfig& config) {
      config.schema = node["schema"].as<silo::config::DatabaseSchema>();
      return true;
   }
};

template <>
struct convert<silo::config::DatabaseSchema> {
   static bool decode(const Node& node, silo::config::DatabaseSchema& schema) {
      schema.instance_name = node["instanceName"].as<std::string>();
      schema.primary_key = node["primaryKey"].as<std::string>();
      if (node["dateToSortBy"].IsDefined()) {
         schema.date_to_sort_by = node["dateToSortBy"].as<std::string>();
      } else {
         schema.date_to_sort_by = std::nullopt;
      }
      schema.partition_by = node["partitionBy"].as<std::string>();

      if (!node["metadata"].IsSequence()) {
         return false;
      }

      for (const auto& metadata : node["metadata"]) {
         schema.metadata.push_back(metadata.as<silo::config::DatabaseMetadata>());
      }
      return true;
   }
};

template <>
struct convert<silo::config::DatabaseMetadata> {
   static bool decode(const Node& node, silo::config::DatabaseMetadata& metadata) {
      metadata.name = node["name"].as<std::string>();
      metadata.type = silo::config::toDatabaseMetadataType(node["type"].as<std::string>());
      if (node["generateIndex"].IsDefined()) {
         metadata.generate_index = node["generateIndex"].as<bool>();
      } else {
         metadata.generate_index =
            metadata.type == silo::config::DatabaseMetadataType::PANGOLINEAGE;
      }
      return true;
   }
};

}  // namespace YAML

namespace silo::config {
DatabaseConfig DatabaseConfigReader::readConfig(const std::filesystem::path& config_path) const {
   return YAML::LoadFile(config_path.string()).as<DatabaseConfig>();
}
}  // namespace silo::config
