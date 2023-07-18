#include "silo/config/database_config.h"

#include <fmt/format.h>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <string_view>

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

[[maybe_unused]] auto fmt::formatter<silo::config::DatabaseConfig>::format(
   const silo::config::DatabaseConfig& database_config,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   return format_to(
      ctx.out(),
      "{{ default_nucleotide_sequence: '{}', schema: {} }}",
      database_config.default_nucleotide_sequence,
      database_config.schema
   );
}

[[maybe_unused]] auto fmt::formatter<silo::config::DatabaseSchema>::format(
   const silo::config::DatabaseSchema& database_schema,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   return format_to(
      ctx.out(),
      "{{ instance_name: '{}', primary_key: '{}', partition_by: '{}', metadata: [{}] }}",
      database_schema.instance_name,
      database_schema.primary_key,
      database_schema.partition_by,
      fmt::join(database_schema.metadata, ",")
   );
}

[[maybe_unused]] auto fmt::formatter<silo::config::DatabaseMetadata>::format(
   const silo::config::DatabaseMetadata& database_metadata,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   return format_to(
      ctx.out(),
      "{{ name: '{}', type: '{}', generate_index: {} }}",
      database_metadata.name,
      database_metadata.type,
      database_metadata.generate_index
   );
}

[[maybe_unused]] auto fmt::formatter<silo::config::ValueType>::format(
   const silo::config::ValueType& value_type,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   switch (value_type) {
      case silo::config::ValueType::STRING:
         return format_to(ctx.out(), "string");
      case silo::config::ValueType::DATE:
         return format_to(ctx.out(), "date");
      case silo::config::ValueType::PANGOLINEAGE:
         return format_to(ctx.out(), "pango_lineage");
      case silo::config::ValueType::INT:
         return format_to(ctx.out(), "int");
      case silo::config::ValueType::FLOAT:
         return format_to(ctx.out(), "float");
   }
   return format_to(ctx.out(), "unknown");
}
