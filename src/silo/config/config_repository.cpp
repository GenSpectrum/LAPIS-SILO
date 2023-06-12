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

std::map<std::string, DatabaseMetadataType> validateMetadataDefinitions(const DatabaseConfig& config
) {
   std::map<std::string, DatabaseMetadataType> metadata_map;
   for (const auto& metadata : config.schema.metadata) {
      if (metadata_map.find(metadata.name) != metadata_map.end()) {
         throw ConfigException("Metadata " + metadata.name + " is defined twice in the config");
      }

      const auto must_not_generate_index_on_type =
         metadata.type != DatabaseMetadataType::STRING &&
         metadata.type != DatabaseMetadataType::PANGOLINEAGE;
      if (metadata.generate_index && must_not_generate_index_on_type) {
         throw ConfigException(
            "Metadata '" + metadata.name +
            "' generate_index is set, but generating an index is only allowed for types STRING and "
            "PANGOLINEAGE"
         );
      }
      metadata_map[metadata.name] = metadata.type;
   }
   return metadata_map;
}

void validateDateToSortBy(
   const DatabaseConfig& config,
   std::map<std::string, DatabaseMetadataType>& metadata_map
) {
   if (!config.schema.date_to_sort_by.has_value()) {
      return;
   }

   const std::string date_to_sort_by = config.schema.date_to_sort_by.value();
   if (metadata_map.find(date_to_sort_by) == metadata_map.end()) {
      throw ConfigException("date_to_sort_by '" + date_to_sort_by + "' is not in metadata");
   }

   if (metadata_map[date_to_sort_by] != DatabaseMetadataType::DATE) {
      throw ConfigException("date_to_sort_by '" + date_to_sort_by + "' must be of type DATE");
   }
}

void validatePartitionBy(
   const DatabaseConfig& config,
   std::map<std::string, DatabaseMetadataType>& metadata_map
) {
   if (metadata_map.find(config.schema.partition_by) == metadata_map.end()) {
      throw ConfigException("partition_by '" + config.schema.partition_by + "' is not in metadata");
   }

   const auto& partition_by_type = metadata_map[config.schema.partition_by];
   if (partition_by_type != DatabaseMetadataType::STRING && partition_by_type != DatabaseMetadataType::PANGOLINEAGE) {
      throw ConfigException(
         "partition_by '" + config.schema.partition_by + "' must be of type STRING or PANGOLINEAGE"
      );
   }
}

void ConfigRepository::validateConfig(const DatabaseConfig& config) const {
   std::map<std::string, DatabaseMetadataType> metadata_map = validateMetadataDefinitions(config);

   if (metadata_map.find(config.schema.primary_key) == metadata_map.end()) {
      throw ConfigException("Primary key is not in metadata");
   }

   validateDateToSortBy(config, metadata_map);

   validatePartitionBy(config, metadata_map);
}

}  // namespace silo::config