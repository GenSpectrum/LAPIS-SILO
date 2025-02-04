#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>
#include <yaml-cpp/yaml.h>

namespace silo::config {

enum class ValueType { STRING, DATE, BOOL, INT, FLOAT };
enum class ColumnType { STRING, INDEXED_STRING, DATE, BOOL, INT, FLOAT };

ValueType toDatabaseValueType(std::string_view type);

class DatabaseMetadata {
  public:
   std::string name;
   ValueType type;
   bool generate_index;
   bool generate_lineage_index;

   [[nodiscard]] ColumnType getColumnType() const;
};

class DatabaseSchema {
  public:
   std::string instance_name;
   std::vector<DatabaseMetadata> metadata;
   std::string primary_key;
   std::optional<std::string> date_to_sort_by;
   std::optional<std::string> partition_by;
};

class DatabaseConfigReader;

class DatabaseConfig {
   friend struct YAML::as_if<DatabaseConfig, void>;

   DatabaseConfig() = default;

  public:
   DatabaseConfig(const DatabaseConfig&) = default;
   DatabaseConfig(DatabaseConfig&&) = default;
   DatabaseConfig& operator=(const DatabaseConfig&) = default;
   DatabaseConfig& operator=(DatabaseConfig&&) = default;

   std::optional<std::string> default_nucleotide_sequence;
   std::optional<std::string> default_amino_acid_sequence;
   DatabaseSchema schema;

   [[nodiscard]] std::optional<DatabaseMetadata> getMetadata(const std::string& name) const;

   void writeConfig(const std::filesystem::path& config_path) const;
};

class DatabaseConfigReader {
  public:
   [[nodiscard]] virtual DatabaseConfig readConfig(const std::filesystem::path& config_path) const;

   [[nodiscard]] virtual DatabaseConfig parseYaml(const std::string& yaml) const;
};

}  // namespace silo::config

namespace YAML {
template <>
struct convert<silo::config::DatabaseConfig> {
   static bool decode(const Node& node, silo::config::DatabaseConfig& config);
   static Node encode(const silo::config::DatabaseConfig& config);
};

template <>
struct convert<silo::config::DatabaseSchema> {
   static bool decode(const Node& node, silo::config::DatabaseSchema& schema);
   static Node encode(const silo::config::DatabaseSchema& schema);
};

template <>
struct convert<silo::config::DatabaseMetadata> {
   static bool decode(const Node& node, silo::config::DatabaseMetadata& metadata);
   static Node encode(const silo::config::DatabaseMetadata& metadata);
};

}  // namespace YAML

template <>
class [[maybe_unused]] fmt::formatter<silo::config::DatabaseConfig> : fmt::formatter<std::string> {
  public:
   constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
   [[maybe_unused]] static auto format(
      const silo::config::DatabaseConfig& database_config,
      fmt::format_context& ctx
   ) -> decltype(ctx.out());
};

template <>
class [[maybe_unused]] fmt::formatter<silo::config::DatabaseSchema> {
  public:
   constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
   [[maybe_unused]] static auto format(
      const silo::config::DatabaseSchema& database_schema,
      fmt::format_context& ctx
   ) -> decltype(ctx.out());
};

template <>
class [[maybe_unused]] fmt::formatter<silo::config::DatabaseMetadata> {
  public:
   constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
   [[maybe_unused]] static auto format(
      const silo::config::DatabaseMetadata& database_metadata,
      format_context& ctx
   ) -> decltype(ctx.out());
};

template <>
class [[maybe_unused]] fmt::formatter<silo::config::ValueType> {
  public:
   constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
   [[maybe_unused]] static auto format(
      const silo::config::ValueType& value_type,
      format_context& ctx
   ) -> decltype(ctx.out());
};
