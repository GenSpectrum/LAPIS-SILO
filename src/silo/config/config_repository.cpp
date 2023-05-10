#include "silo/config/config_repository.h"

#include <map>
#include <string>

#include "silo/config/config_exception.h"
#include "silo/config/database_config_reader.h"

namespace silo::config {

ConfigRepository::ConfigRepository(const DatabaseConfigReader& reader)
    : reader_(reader) {}

DatabaseConfig ConfigRepository::readConfig(const std::filesystem::path& config_path) const {
   auto config = reader_.readConfig(config_path);
   validateConfig(config);
   return config;
}

void ConfigRepository::validateConfig(const DatabaseConfig& config) const {
   std::map<std::string, DatabaseMetadataType> metadata_map;
   for (const auto& metadata : config.schema.metadata) {
      if (metadata_map.find(metadata.name) != metadata_map.end()) {
         throw ConfigException("Metadata " + metadata.name + " is defined twice in the config");
      }
      metadata_map[metadata.name] = metadata.type;
   }

   if (metadata_map.find(config.schema.primary_key) == metadata_map.end()) {
      throw ConfigException("Primary key is not in metadata");
   }
}

std::string ConfigRepository::getPrimaryKey(const std::filesystem::path& path) const {
   const auto config = readConfig(path);
   return config.schema.primary_key;
}

DatabaseMetadata ConfigRepository::getMetadata(
   const std::filesystem::path& path,
   const std::string& name
) const {
   const auto config = readConfig(path);
   for (const auto& metadata : config.schema.metadata) {
      if (metadata.name == name) {
         return metadata;
      }
   }
   throw ConfigException("Metadata with name " + name + " not found");
}

}  // namespace silo::config