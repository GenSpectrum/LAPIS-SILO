#ifndef SILO_COLUMN_GROUP_H
#define SILO_COLUMN_GROUP_H

#include <filesystem>
#include <vector>

#include "silo/storage/column/date_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/pango_lineage_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/sequence_store.h"

namespace silo::config {
struct DatabaseMetadata;
}

namespace silo {
class PangoLineageAliasLookup;

namespace config {
class DatabaseConfig;
}  // namespace config
}  // namespace silo

namespace silo::storage {

struct ColumnGroup {
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      // clang-format on
   }

   std::vector<config::DatabaseMetadata> metadata;

   std::unordered_map<std::string, storage::column::StringColumnPartition&> string_columns;
   std::unordered_map<std::string, storage::column::IndexedStringColumnPartition&>
      indexed_string_columns;
   std::unordered_map<std::string, storage::column::IntColumnPartition&> int_columns;
   std::unordered_map<std::string, storage::column::FloatColumnPartition&> float_columns;
   std::unordered_map<std::string, storage::column::DateColumnPartition&> date_columns;
   std::unordered_map<std::string, storage::column::PangoLineageColumnPartition&>
      pango_lineage_columns;

   uint32_t fill(
      const std::filesystem::path& input_file,
      const PangoLineageAliasLookup& alias_key,
      const silo::config::DatabaseConfig& database_config
   );

   ColumnGroup getSubgroup(const std::vector<config::DatabaseMetadata>& fields) const;
};

}  // namespace silo::storage

#endif  // SILO_COLUMN_GROUP_H
