#include "silo/config/database_config.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string_view>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include "config/config_exception.h"

using silo::config::ValueType;

ValueType silo::config::toDatabaseValueType(std::string_view type) {
   if (type == "string") {
      return ValueType::STRING;
   }
   if (type == "date") {
      return ValueType::DATE;
   }
   if (type == "boolean") {
      return ValueType::BOOL;
   }
   if (type == "int") {
      return ValueType::INT;
   }
   if (type == "float") {
      return ValueType::FLOAT;
   }

   throw silo::config::ConfigException("Unknown metadata type: " + std::string(type));
}

namespace {

const std::string DEFAULT_NUCLEOTIDE_SEQUENCE_KEY = "defaultNucleotideSequence";
const std::string DEFAULT_AMINO_ACID_SEQUENCE_KEY = "defaultAminoAcidSequence";

std::string toString(ValueType type) {
   switch (type) {
      case ValueType::STRING:
         return "string";
      case ValueType::DATE:
         return "date";
      case ValueType::BOOL:
         return "boolean";
      case ValueType::INT:
         return "int";
      case ValueType::FLOAT:
         return "float";
   }
   throw std::runtime_error("Non-exhausting switch should be covered by linter");
}
}  // namespace

bool YAML::convert<silo::config::DatabaseConfig>::decode(
   const Node& node,
   silo::config::DatabaseConfig& config
) {
   config.schema = node["schema"].as<silo::config::DatabaseSchema>();

   if (node[DEFAULT_NUCLEOTIDE_SEQUENCE_KEY].IsDefined() && !node[DEFAULT_NUCLEOTIDE_SEQUENCE_KEY].IsNull()) {
      config.default_nucleotide_sequence = node[DEFAULT_NUCLEOTIDE_SEQUENCE_KEY].as<std::string>();
   }
   if (node[DEFAULT_AMINO_ACID_SEQUENCE_KEY].IsDefined() && !node[DEFAULT_AMINO_ACID_SEQUENCE_KEY].IsNull()) {
      config.default_amino_acid_sequence = node[DEFAULT_AMINO_ACID_SEQUENCE_KEY].as<std::string>();
   }

   SPDLOG_TRACE("Resulting database config: {}", config);

   return true;
}
YAML::Node YAML::convert<silo::config::DatabaseConfig>::encode(
   const silo::config::DatabaseConfig& config
) {
   Node node;
   node["schema"] = config.schema;

   if (config.default_nucleotide_sequence.has_value()) {
      node[DEFAULT_NUCLEOTIDE_SEQUENCE_KEY] = *config.default_nucleotide_sequence;
   }
   if (config.default_amino_acid_sequence.has_value()) {
      node[DEFAULT_AMINO_ACID_SEQUENCE_KEY] = *config.default_amino_acid_sequence;
   }
   return node;
}

bool YAML::convert<silo::config::DatabaseSchema>::decode(
   const Node& node,
   silo::config::DatabaseSchema& schema
) {
   schema.instance_name = node["instanceName"].as<std::string>();
   schema.primary_key = node["primaryKey"].as<std::string>();
   // DEPRECATED: TODO(#737) fully remove them after the next major release
   if (node["dateToSortBy"].IsDefined()) {
      SPDLOG_WARN(
         "DatabaseConfig field `dateToSortBy` is deprecated and will be removed in future releases."
      );
   }
   if (node["partitionBy"].IsDefined()) {
      SPDLOG_WARN(
         "DatabaseConfig field `partitionBy` is deprecated and will be removed in future releases."
      );
   }

   if (!node["metadata"].IsSequence()) {
      return false;
   }

   for (const auto& metadata : node["metadata"]) {
      schema.metadata.push_back(metadata.as<silo::config::DatabaseMetadata>());
   }
   return true;
}

YAML::Node YAML::convert<silo::config::DatabaseSchema>::encode(
   const silo::config::DatabaseSchema& schema
) {
   Node node;
   node["instanceName"] = schema.instance_name;
   node["primaryKey"] = schema.primary_key;
   node["metadata"] = schema.metadata;
   return node;
}

bool YAML::convert<silo::config::DatabaseMetadata>::decode(
   const Node& node,
   silo::config::DatabaseMetadata& metadata
) {
   metadata.name = node["name"].as<std::string>();
   metadata.type = silo::config::toDatabaseValueType(node["type"].as<std::string>());
   if (node["generateIndex"].IsDefined()) {
      metadata.generate_index = node["generateIndex"].as<bool>();
   } else {
      metadata.generate_index = false;
   }
   if (node["generateLineageIndex"].IsDefined()) {
      metadata.generate_lineage_index = node["generateLineageIndex"].as<bool>();
   } else {
      metadata.generate_lineage_index = false;
   }
   if (node["generatePhyloTreeIndex"].IsDefined()) {
      metadata.generate_phylo_tree_index = node["generatePhyloTreeIndex"].as<bool>();
   } else {
      metadata.generate_phylo_tree_index = false;
   }
   return true;
}
YAML::Node YAML::convert<silo::config::DatabaseMetadata>::encode(
   const silo::config::DatabaseMetadata& metadata
) {
   Node node;
   node["name"] = metadata.name;
   node["type"] = toString(metadata.type);
   node["generateIndex"] = metadata.generate_index;
   if (metadata.generate_lineage_index) {
      node["generateLineageIndex"] = true;
   }
   if (metadata.generate_phylo_tree_index) {
      node["generatePhyloTreeIndex"] = true;
   }
   return node;
}

namespace silo::config {

schema::ColumnType DatabaseMetadata::getColumnType() const {
   if (type == ValueType::STRING) {
      if (generate_index) {
         return schema::ColumnType::INDEXED_STRING;
      }
      return schema::ColumnType::STRING;
   }
   if (type == ValueType::DATE) {
      return schema::ColumnType::DATE;
   }
   if (type == ValueType::BOOL) {
      return schema::ColumnType::BOOL;
   }
   if (type == ValueType::INT) {
      return schema::ColumnType::INT;
   }
   if (type == ValueType::FLOAT) {
      return schema::ColumnType::FLOAT;
   }
   throw std::runtime_error("Did not find metadata with name: " + std::string(name));
}

std::optional<DatabaseMetadata> DatabaseConfig::getMetadata(const std::string& name) const {
   auto element = std::ranges::find_if(schema.metadata, [&name](const auto& metadata) {
      return metadata.name == name;
   });
   if (element == std::end(schema.metadata)) {
      return std::nullopt;
   }
   return *element;
}

void DatabaseConfig::writeConfig(const std::filesystem::path& config_path) const {
   const YAML::Node node = YAML::convert<DatabaseConfig>::encode(*this);
   SPDLOG_DEBUG("Writing database config to {}", config_path.string());
   std::ofstream out_file(config_path);
   out_file << YAML::Dump(node);
}

DatabaseConfig DatabaseConfig::getValidatedConfig(const std::string& config_yaml) {
   auto config = YAML::Load(config_yaml).as<DatabaseConfig>();
   validateConfig(config);
   return config;
}

DatabaseConfig DatabaseConfig::getValidatedConfigFromFile(const std::filesystem::path& config_path
) {
   SPDLOG_INFO("Reading database config from {}", config_path.string());
   std::stringstream yaml;

   std::ifstream file(config_path);
   if (!file.is_open()) {
      throw std::runtime_error(
         "Failed to read database config: Could not open file " + config_path.string()
      );
   }

   if (file.peek() != std::ifstream::traits_type::eof()) {
      yaml << file.rdbuf();
   }

   try {
      return DatabaseConfig::getValidatedConfig(yaml.str());
   } catch (const YAML::Exception& e) {
      throw std::runtime_error(
         "Failed to read database config from " + config_path.string() + ": " +
         std::string(e.what())
      );
   }
}

namespace {
std::map<std::string, ValueType> validateMetadataDefinitions(const DatabaseConfig& config) {
   std::map<std::string, ValueType> metadata_map;
   for (const auto& metadata : config.schema.metadata) {
      if (metadata_map.find(metadata.name) != metadata_map.end()) {
         throw ConfigException("Metadata " + metadata.name + " is defined twice in the config");
      }

      const auto generate_lineage_indexed_field = metadata.generate_lineage_index;
      if (metadata.type != ValueType::STRING && generate_lineage_indexed_field) {
         throw ConfigException(
            "Metadata '" + metadata.name +
            "' generateLineageIndex is set, but the column is not of type STRING."
         );
      }

      const auto generate_phylo_tree_indexed_field = metadata.generate_phylo_tree_index;
      if (metadata.type != ValueType::STRING && generate_phylo_tree_indexed_field) {
         throw ConfigException(
            "Metadata '" + metadata.name +
            "' generatePhyloTreeIndex is set, but the column is not of type STRING."
         );
      }

      const auto must_not_generate_index_on_type = metadata.type != ValueType::STRING;
      if (metadata.generate_index && must_not_generate_index_on_type) {
         throw ConfigException(
            "Metadata '" + metadata.name +
            "' generateIndex is set, but generating an index is only allowed for types STRING"
         );
      }

      if (!metadata.generate_index && generate_lineage_indexed_field) {
         throw ConfigException(
            "Metadata '" + metadata.name +
            "' generateLineageIndex is set, generateIndex must also be set."
         );
      }

      if (metadata.generate_index && generate_phylo_tree_indexed_field) {
         throw ConfigException(
            "Metadata '" + metadata.name +
            "' generatePhyloTreeIndex and generateIndex are both set, if generatePhyloTreeIndex is "
            "set then generateIndex cannot be set."
         );
      }

      metadata_map[metadata.name] = metadata.type;
   }
   return metadata_map;
}
}  // namespace

void DatabaseConfig::validateConfig(const DatabaseConfig& config) {
   std::map<std::string, ValueType> metadata_map = validateMetadataDefinitions(config);

   if (config.schema.metadata.empty()) {
      throw ConfigException("Database config without fields not possible");
   }

   if (metadata_map.find(config.schema.primary_key) == metadata_map.end()) {
      throw ConfigException("Primary key is not in metadata");
   }
}

}  // namespace silo::config

[[maybe_unused]] auto fmt::formatter<silo::config::DatabaseConfig>::format(
   const silo::config::DatabaseConfig& database_config,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   return fmt::format_to(
      ctx.out(),
      "{{ default_nucleotide_sequence: {}, default_amino_acid_sequence: {}, schema: {} }}",
      database_config.default_nucleotide_sequence.has_value()
         ? "'" + *database_config.default_nucleotide_sequence + "'"
         : "null",
      database_config.default_amino_acid_sequence.has_value()
         ? "'" + *database_config.default_nucleotide_sequence + "'"
         : "null",
      database_config.schema
   );
}

[[maybe_unused]] auto fmt::formatter<silo::config::DatabaseSchema>::format(
   const silo::config::DatabaseSchema& database_schema,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   return fmt::format_to(
      ctx.out(),
      "{{ instance_name: '{}', primary_key: '{}', metadata: [{}] }}",
      database_schema.instance_name,
      database_schema.primary_key,
      fmt::join(database_schema.metadata, ",")
   );
}

[[maybe_unused]] auto fmt::formatter<silo::config::DatabaseMetadata>::format(
   const silo::config::DatabaseMetadata& database_metadata,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   return fmt::format_to(
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
         return fmt::format_to(ctx.out(), "string");
      case silo::config::ValueType::DATE:
         return fmt::format_to(ctx.out(), "date");
      case silo::config::ValueType::BOOL:
         return fmt::format_to(ctx.out(), "bool");
      case silo::config::ValueType::INT:
         return fmt::format_to(ctx.out(), "int");
      case silo::config::ValueType::FLOAT:
         return fmt::format_to(ctx.out(), "float");
   }
   return fmt::format_to(ctx.out(), "unknown");
}
