#ifndef SILO_INCLUDE_SILO_CONFIG_DATABASECONFIG_H_
#define SILO_INCLUDE_SILO_CONFIG_DATABASECONFIG_H_

#include <optional>
#include <string>
#include <vector>

namespace silo::config {

enum class DatabaseMetadataType { STRING, PANGOLINEAGE, DATE };

DatabaseMetadataType toDatabaseMetadataType(std::string_view type);

struct DatabaseMetadata {
   std::string name;
   DatabaseMetadataType type;
   bool generate_index;
};

struct DatabaseSchema {
   std::string instance_name;
   std::vector<DatabaseMetadata> metadata;
   std::string primary_key;
   std::optional<std::string> date_to_sort_by;
   std::string partition_by;
};

struct DatabaseConfig {
   DatabaseSchema schema;

   DatabaseMetadata getMetadata(const std::string& name) const;
};
}  // namespace silo::config

#endif  // SILO_INCLUDE_SILO_CONFIG_DATABASECONFIG_H_
