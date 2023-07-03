#ifndef SILO_INCLUDE_SILO_CONFIG_DATABASECONFIG_H_
#define SILO_INCLUDE_SILO_CONFIG_DATABASECONFIG_H_

#include <optional>
#include <string>
#include <vector>

namespace silo::config {

enum class ValueType { STRING, PANGOLINEAGE, DATE, INT, FLOAT };
enum class ColumnType { STRING, INDEXED_STRING, INDEXED_PANGOLINEAGE, DATE, INT, FLOAT };

ValueType toDatabaseValueType(std::string_view type);

struct DatabaseMetadata {
   std::string name;
   ValueType type;
   bool generate_index;

   [[nodiscard]] ColumnType getColumnType() const;
};

struct DatabaseSchema {
   std::string instance_name;
   std::vector<DatabaseMetadata> metadata;
   std::string primary_key;
   std::optional<std::string> date_to_sort_by;
   std::string partition_by;
};

struct DatabaseConfig {
   std::string default_nucleotide_sequence;
   DatabaseSchema schema;

   [[nodiscard]] DatabaseMetadata getMetadata(const std::string& name) const;
};
}  // namespace silo::config

#endif  // SILO_INCLUDE_SILO_CONFIG_DATABASECONFIG_H_
