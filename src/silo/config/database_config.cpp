#include "silo/config/database_config.h"

#include <algorithm>

#include "silo/config/config_exception.h"

namespace silo::config {

ValueType toDatabaseValueType(std::string_view type) {
   if (type == "string") {
      return ValueType::STRING;
   }
   if (type == "date") {
      return ValueType::DATE;
   }
   if (type == "pango_lineage") {
      return ValueType::PANGOLINEAGE;
   }
   if (type == "int") {
      return ValueType::INT;
   }
   if (type == "float") {
      return ValueType::FLOAT;
   }

   throw ConfigException("Unknown metadata type: " + std::string(type));
}

ColumnType DatabaseMetadata::getColumnType() const {
   if (type == ValueType::STRING) {
      if (generate_index) {
         return ColumnType::INDEXED_STRING;
      }
      return ColumnType::STRING;
   }
   if (type == ValueType::DATE) {
      return ColumnType::DATE;
   }
   if (type == ValueType::PANGOLINEAGE) {
      if (generate_index) {
         return ColumnType::INDEXED_PANGOLINEAGE;
      }
      throw std::runtime_error("Found pango lineage column without index: " + std::string(name));
   }
   if (type == ValueType::INT) {
      return ColumnType::INT;
   }
   if (type == ValueType::FLOAT) {
      return ColumnType::FLOAT;
   }

   throw std::runtime_error("Did not find metadata with name: " + std::string(name));
}

std::optional<DatabaseMetadata> DatabaseConfig::getMetadata(const std::string& name) const {
   auto element = std::find_if(
      std::begin(schema.metadata),
      std::end(schema.metadata),
      [&name](const auto& metadata) { return metadata.name == name; }
   );
   if (element == std::end(schema.metadata)) {
      return std::nullopt;
   }
   return *element;
}

}  // namespace silo::config