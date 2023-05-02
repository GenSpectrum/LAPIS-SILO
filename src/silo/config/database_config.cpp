#include "silo/config/database_config.h"

#include "silo/config/config_exception.h"

namespace silo::config {

DatabaseMetadataType toDatabaseMetadataType(std::string_view type) {
   if (type == "string") {
      return DatabaseMetadataType::STRING;
   }
   if (type == "date") {
      return DatabaseMetadataType::DATE;
   }
   if (type == "pango_lineage") {
      return DatabaseMetadataType::PANGOLINEAGE;
   }

   throw ConfigException("Unknown metadata type: " + std::string(type));
}

}  // namespace silo::config