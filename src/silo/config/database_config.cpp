#include "silo/config/database_config.h"

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <stdexcept>
#include <string_view>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include <fstream>

#include "silo/config/config_exception.h"

using silo::config::ValueType;

ValueType silo::config::toDatabaseValueType(std::string_view type) {
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
   if (type == "insertion") {
      return ValueType::INSERTION;
   }
   if (type == "aaInsertion") {
      return ValueType::AA_INSERTION;
   }

   throw silo::config::ConfigException("Unknown metadata type: " + std::string(type));
}

namespace {

std::string toString(ValueType type) {
   switch (type) {
      case ValueType::STRING:
         return "string";
      case ValueType::DATE:
         return "date";
      case ValueType::PANGOLINEAGE:
         return "pango_lineage";
      case ValueType::INT:
         return "int";
      case ValueType::FLOAT:
         return "float";
      case ValueType::INSERTION:
         return "insertion";
      case ValueType::AA_INSERTION:
         return "aaInsertion";
   }
   throw std::runtime_error("Non-exhausting switch should be covered by linter");
}
}  // namespace

namespace YAML {
template <>
struct convert<silo::config::DatabaseConfig> {
   static bool decode(const Node& node, silo::config::DatabaseConfig& config) {
      config.schema = node["schema"].as<silo::config::DatabaseSchema>();

      if (node["defaultNucleotideSequence"].IsDefined()) {
         config.default_nucleotide_sequence = node["defaultNucleotideSequence"].as<std::string>();
      } else {
         config.default_nucleotide_sequence = "main";
      }

      SPDLOG_TRACE("Resulting database config: {}", config);

      return true;
   }
   static Node encode(const silo::config::DatabaseConfig& config) {
      Node node;
      node["schema"] = config.schema;

      if (config.default_nucleotide_sequence != "main") {
         node["defaultNucleotideSequence"] = config.default_nucleotide_sequence;
      }
      return node;
   }
};

template <>
struct convert<silo::config::DatabaseSchema> {
   static bool decode(const Node& node, silo::config::DatabaseSchema& schema) {
      schema.instance_name = node["instanceName"].as<std::string>();
      schema.primary_key = node["primaryKey"].as<std::string>();
      if (node["dateToSortBy"].IsDefined()) {
         schema.date_to_sort_by = node["dateToSortBy"].as<std::string>();
      } else {
         schema.date_to_sort_by = std::nullopt;
      }
      if (node["partitionBy"].IsDefined()) {
         schema.partition_by = node["partitionBy"].as<std::string>();
      } else {
         schema.partition_by = std::nullopt;
      }

      if (!node["metadata"].IsSequence()) {
         return false;
      }

      for (const auto& metadata : node["metadata"]) {
         schema.metadata.push_back(metadata.as<silo::config::DatabaseMetadata>());
      }
      return true;
   }

   static Node encode(const silo::config::DatabaseSchema& schema) {
      Node node;
      node["instanceName"] = schema.instance_name;
      node["primaryKey"] = schema.primary_key;
      if (schema.partition_by.has_value()) {
         node["partitionBy"] = *schema.partition_by;
      }
      if (schema.date_to_sort_by.has_value()) {
         node["dateToSortBy"] = *schema.date_to_sort_by;
      }
      node["metadata"] = schema.metadata;
      return node;
   }
};

template <>
struct convert<silo::config::DatabaseMetadata> {
   static bool decode(const Node& node, silo::config::DatabaseMetadata& metadata) {
      metadata.name = node["name"].as<std::string>();
      metadata.type = silo::config::toDatabaseValueType(node["type"].as<std::string>());
      if (node["generateIndex"].IsDefined()) {
         metadata.generate_index = node["generateIndex"].as<bool>();
      } else {
         metadata.generate_index = metadata.type == silo::config::ValueType::PANGOLINEAGE;
      }
      return true;
   }
   static Node encode(const silo::config::DatabaseMetadata& metadata) {
      Node node;
      node["name"] = metadata.name;
      node["type"] = toString(metadata.type);
      node["generateIndex"] = metadata.generate_index;
      return node;
   }
};

}  // namespace YAML

namespace silo::config {

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
   if (type == ValueType::INSERTION) {
      return ColumnType::INSERTION;
   }
   if (type == ValueType::AA_INSERTION) {
      return ColumnType::AA_INSERTION;
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

void DatabaseConfig::writeConfig(const std::filesystem::path& config_path) const {
   const YAML::Node node = YAML::convert<DatabaseConfig>::encode(*this);
   SPDLOG_INFO("Writing database config to {}", config_path.string());
   std::ofstream out_file(config_path);
   out_file << YAML::Dump(node);
}

DatabaseConfig DatabaseConfigReader::readConfig(const std::filesystem::path& config_path) const {
   SPDLOG_INFO("Reading database config from {}", config_path.string());
   try {
      return YAML::LoadFile(config_path.string()).as<DatabaseConfig>();
   } catch (const YAML::Exception& e) {
      throw std::runtime_error(
         "Failed to read database config from " + config_path.string() + ": " +
         std::string(e.what())
      );
   }
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
      "{{ instance_name: '{}', primary_key: '{}', partition_by: {}, date_to_sort_by: {}, metadata: "
      "[{}] }}",
      database_schema.instance_name,
      database_schema.primary_key,
      database_schema.partition_by.has_value() ? "'" + *database_schema.partition_by + "'" : "none",
      database_schema.date_to_sort_by.has_value() ? "'" + *database_schema.date_to_sort_by + "'"
                                                  : "none",
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
      case silo::config::ValueType::INSERTION:
         return format_to(ctx.out(), "insertion");
      case silo::config::ValueType::AA_INSERTION:
         return format_to(ctx.out(), "aaInsertion");
   }
   return format_to(ctx.out(), "unknown");
}
