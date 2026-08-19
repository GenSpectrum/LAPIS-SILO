#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>
#include <yaml-cpp/yaml.h>

#include "rhydb/schema/database_schema.h"

namespace rhydb::config {

schema::ValueType toDatabaseValueType(std::string_view type);

enum class LineageIndexType : uint8_t { COLUMN_METADATA, TABLE, BOTH };

LineageIndexType toLineageIndexType(std::string_view type);

std::string_view lineageIndexTypeToString(LineageIndexType type);

class DatabaseMetadata {
  public:
   std::string name;
   schema::ValueType type;
   bool generate_index;
   std::optional<std::string> generate_lineage_index;
   LineageIndexType lineage_index_type = LineageIndexType::COLUMN_METADATA;
   bool phylo_tree_node_identifier;
   bool treat_unknown_lineages_as_null;

   [[nodiscard]] schema::ColumnType getColumnType() const;

   /// Whether the column itself should carry the in-memory lineage index (i.e. the lineage tree is
   /// attached to the column metadata). True for `COLUMN_METADATA` and `BOTH`.
   [[nodiscard]] bool generatesLineageColumnIndex() const;

   /// Whether preprocessing should materialize a companion lineage relation table for this column.
   /// True for `TABLE` and `BOTH`.
   [[nodiscard]] bool generatesLineageTable() const;
};

class DatabaseSchema {
  public:
   std::string instance_name;
   std::vector<DatabaseMetadata> metadata;
   std::string primary_key;
};

class DatabaseConfig {
   friend struct YAML::as_if<DatabaseConfig, void>;

   DatabaseConfig() = default;

  public:
   DatabaseConfig(const DatabaseConfig&) = default;
   DatabaseConfig(DatabaseConfig&&) = default;
   DatabaseConfig& operator=(const DatabaseConfig&) = default;
   DatabaseConfig& operator=(DatabaseConfig&&) = default;

   DatabaseSchema schema;

   [[nodiscard]] std::optional<DatabaseMetadata> getMetadata(const std::string& name) const;

   void writeConfig(const std::filesystem::path& config_path) const;

   static DatabaseConfig getValidatedConfig(const std::string& config_yaml);

   static DatabaseConfig getValidatedConfigFromFile(const std::filesystem::path& config_path);

   static void validateConfig(const DatabaseConfig& config);
};

}  // namespace rhydb::config

namespace YAML {
template <>
struct convert<rhydb::config::DatabaseConfig> {
   static bool decode(const Node& node, rhydb::config::DatabaseConfig& config);
   static Node encode(const rhydb::config::DatabaseConfig& config);
};

template <>
struct convert<rhydb::config::DatabaseSchema> {
   static bool decode(const Node& node, rhydb::config::DatabaseSchema& schema);
   static Node encode(const rhydb::config::DatabaseSchema& schema);
};

template <>
struct convert<rhydb::config::DatabaseMetadata> {
   static bool decode(const Node& node, rhydb::config::DatabaseMetadata& metadata);
   static Node encode(const rhydb::config::DatabaseMetadata& metadata);
};

}  // namespace YAML

template <>
class [[maybe_unused]] fmt::formatter<rhydb::config::DatabaseConfig> : fmt::formatter<std::string> {
  public:
   static constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
   [[maybe_unused]] static auto format(
      const rhydb::config::DatabaseConfig& database_config,
      fmt::format_context& ctx
   ) -> decltype(ctx.out());
};

template <>
class [[maybe_unused]] fmt::formatter<rhydb::config::DatabaseSchema> {
  public:
   static constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
   [[maybe_unused]] static auto format(
      const rhydb::config::DatabaseSchema& database_schema,
      fmt::format_context& ctx
   ) -> decltype(ctx.out());
};

template <>
class [[maybe_unused]] fmt::formatter<rhydb::config::DatabaseMetadata> {
  public:
   static constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
   [[maybe_unused]] static auto format(
      const rhydb::config::DatabaseMetadata& database_metadata,
      format_context& ctx
   ) -> decltype(ctx.out());
};
