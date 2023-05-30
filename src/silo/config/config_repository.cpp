#include "silo/config/config_repository.h"

#include <map>
#include <string>

#include "silo/config/config_exception.h"
#include "silo/config/database_config_reader.h"

namespace silo::config {

ConfigRepository::ConfigRepository(const DatabaseConfigReader& reader)
    : reader_(reader) {}

DatabaseConfig ConfigRepository::getValidatedConfig(const std::filesystem::path& path) const {
   auto config = reader_.readConfig(path);
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

}  // namespace silo::config