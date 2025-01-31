#include "silo/config/config_repository.h"

#include <map>
#include <optional>
#include <string>

#include "config/config_exception.h"
#include "silo/config/database_config.h"

namespace silo::config {

ConfigRepository::ConfigRepository(const DatabaseConfigReader& reader)
    : reader_(reader) {}

DatabaseConfig ConfigRepository::getValidatedConfig(const std::filesystem::path& path) const {
   auto config = reader_.readConfig(path);
   validateConfig(config);
   return config;
}

namespace {
std::map<std::string, ValueType> validateMetadataDefinitions(const DatabaseConfig& config) {
   std::map<std::string, ValueType> metadata_map;
   for (const auto& metadata : config.schema.metadata) {
      if (metadata_map.find(metadata.name) != metadata_map.end()) {
         throw ConfigException("Metadata " + metadata.name + " is defined twice in the config");
      }

      const auto must_be_string = metadata.generate_lineage_index;
      if (metadata.type != ValueType::STRING && must_be_string) {
         throw ConfigException(
            "Metadata '" + metadata.name +
            "' generateLineageIndex is set, but the column is not of type STRING."
         );
      }

      const auto must_not_generate_index_on_type = metadata.type != ValueType::STRING;
      if (metadata.generate_index && must_not_generate_index_on_type) {
         throw ConfigException(
            "Metadata '" + metadata.name +
            "' generateIndex is set, but generating an index is only allowed for types STRING"
         );
      }

      const auto must_generate_index = metadata.generate_lineage_index;
      if (!metadata.generate_index && must_generate_index) {
         throw ConfigException(
            "Metadata '" + metadata.name +
            "' generateLineageIndex is set, generateIndex must also be set."
         );
      }

      metadata_map[metadata.name] = metadata.type;
   }
   return metadata_map;
}

void validateDateToSortBy(
   const DatabaseConfig& config,
   std::map<std::string, ValueType>& metadata_map
) {
   if (!config.schema.date_to_sort_by.has_value()) {
      return;
   }

   const std::string date_to_sort_by = config.schema.date_to_sort_by.value();
   if (metadata_map.find(date_to_sort_by) == metadata_map.end()) {
      throw ConfigException("dateToSortBy '" + date_to_sort_by + "' is not in metadata");
   }

   if (metadata_map[date_to_sort_by] != ValueType::DATE) {
      throw ConfigException("dateToSortBy '" + date_to_sort_by + "' must be of type DATE");
   }
}

void validatePartitionBy(const DatabaseConfig& config) {
   if (config.schema.partition_by == std::nullopt) {
      return;
   }

   const std::string partition_by = config.schema.partition_by.value();

   const auto& partition_by_metadata =
      std::ranges::find_if(config.schema.metadata, [&](const DatabaseMetadata& metadata) {
         return metadata.name == partition_by;
      });
   if (partition_by_metadata == config.schema.metadata.end()) {
      throw ConfigException("partitionBy '" + partition_by + "' is not in metadata");
   }

   if (partition_by_metadata->type != ValueType::STRING || !partition_by_metadata->generate_lineage_index) {
      throw ConfigException(
         "partitionBy '" + partition_by +
         "' must be of type STRING and needs 'generateLineageIndex' set"
      );
   }
}
}  // namespace

void ConfigRepository::validateConfig(const DatabaseConfig& config) const {
   std::map<std::string, ValueType> metadata_map = validateMetadataDefinitions(config);

   if (config.schema.metadata.empty()) {
      throw ConfigException("Database config without fields not possible");
   }

   if (metadata_map.find(config.schema.primary_key) == metadata_map.end()) {
      throw ConfigException("Primary key is not in metadata");
   }

   validateDateToSortBy(config, metadata_map);

   validatePartitionBy(config);
}

}  // namespace silo::config