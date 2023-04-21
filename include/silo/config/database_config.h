#ifndef SILO_INCLUDE_SILO_CONFIG_DATABASECONFIG_H_
#define SILO_INCLUDE_SILO_CONFIG_DATABASECONFIG_H_

#include <string>
#include <vector>

namespace silo {
struct DatabaseMetadata {
   std::string name;
   std::string type;
};

struct DatabaseSchema {
   std::string instance_name;
   std::vector<DatabaseMetadata> metadata;
   std::string primary_key;
};

struct DatabaseConfig {
   DatabaseSchema schema;
};
}  // namespace silo

#endif  // SILO_INCLUDE_SILO_CONFIG_DATABASECONFIG_H_
