#ifndef SILO_INCLUDE_SILO_CONFIG_DATABASECONFIG_H_
#define SILO_INCLUDE_SILO_CONFIG_DATABASECONFIG_H_

#include <string>
#include <vector>

namespace silo::config {

enum class DatabaseMetadataType { STRING, PANGOLINEAGE, DATE };

DatabaseMetadataType toDatabaseMetadataType(std::string_view type);

struct DatabaseMetadata {
   std::string name;
   DatabaseMetadataType type;
};

struct DatabaseSchema {
   std::string instance_name;
   std::vector<DatabaseMetadata> metadata;
   std::string primary_key;
};

struct DatabaseConfig {
   DatabaseSchema schema;
};
}  // namespace silo::config

#endif  // SILO_INCLUDE_SILO_CONFIG_DATABASECONFIG_H_
