#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/core.h>

namespace silo::config {

enum class ValueType { STRING, PANGOLINEAGE, DATE, BOOL, INT, FLOAT, NUC_INSERTION, AA_INSERTION };
enum class ColumnType {
   STRING,
   INDEXED_STRING,
   INDEXED_PANGOLINEAGE,
   DATE,
   BOOL,
   INT,
   FLOAT,
   NUC_INSERTION,
   AA_INSERTION
};

ValueType toDatabaseValueType(std::string_view type);

class DatabaseMetadata {
  public:
   std::string name;
   ValueType type;
   bool generate_index;

   [[nodiscard]] ColumnType getColumnType() const;
};

class DatabaseSchema {
  public:
   std::string instance_name;
   std::vector<DatabaseMetadata> metadata;
   std::string primary_key;
   std::optional<std::string> date_to_sort_by;
   std::optional<std::string> partition_by;

   [[nodiscard]] std::string getStrictOrderByClause() const;
};

class DatabaseConfig {
  public:
   std::string default_nucleotide_sequence;
   DatabaseSchema schema;

   [[nodiscard]] std::optional<DatabaseMetadata> getMetadata(const std::string& name) const;

   void writeConfig(const std::filesystem::path& config_path) const;
};

class DatabaseConfigReader {
  public:
   [[nodiscard]] virtual DatabaseConfig readConfig(const std::filesystem::path& config_path) const;
};

}  // namespace silo::config

template <>
struct [[maybe_unused]] fmt::formatter<silo::config::DatabaseConfig> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const silo::config::DatabaseConfig& database_config,
      format_context& ctx
   ) -> decltype(ctx.out());
};

template <>
struct [[maybe_unused]] fmt::formatter<silo::config::DatabaseSchema> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const silo::config::DatabaseSchema& database_schema,
      format_context& ctx
   ) -> decltype(ctx.out());
};

template <>
struct [[maybe_unused]] fmt::formatter<silo::config::DatabaseMetadata>
    : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const silo::config::DatabaseMetadata& database_metadata,
      format_context& ctx
   ) -> decltype(ctx.out());
};

template <>
struct [[maybe_unused]] fmt::formatter<silo::config::ValueType> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const silo::config::ValueType& value_type,
      format_context& ctx
   ) -> decltype(ctx.out());
};
