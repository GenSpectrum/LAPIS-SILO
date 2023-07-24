#ifndef SILO_COLUMN_GROUP_H
#define SILO_COLUMN_GROUP_H

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "silo/config/database_config.h"
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

namespace storage {
namespace column {
class DateColumnPartition;
class FloatColumnPartition;
class IndexedStringColumnPartition;
class IntColumnPartition;
class PangoLineageColumnPartition;
class StringColumnPartition;
}  // namespace column
}  // namespace storage

namespace config {
class DatabaseConfig;
}  // namespace config
}  // namespace silo

namespace silo::storage {

struct ColumnPartitionGroup {
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      for(auto& [name, store] : string_columns){
         archive & store;
      }
      for(auto& [name, store] : indexed_string_columns){
         archive & store;
      }
      for(auto& [name, store] : int_columns){
         archive & store;
      }
      for(auto& [name, store] : float_columns){
         archive & store;
      }
      for(auto& [name, store] : date_columns){
         archive & store;
      }
      for(auto& [name, store] : pango_lineage_columns){
         archive & store;
      }
      // clang-format on
   }

   std::vector<config::DatabaseMetadata> metadata;

   std::map<std::string, storage::column::StringColumnPartition&> string_columns;
   std::map<std::string, storage::column::IndexedStringColumnPartition&> indexed_string_columns;
   std::map<std::string, storage::column::IntColumnPartition&> int_columns;
   std::map<std::string, storage::column::FloatColumnPartition&> float_columns;
   std::map<std::string, storage::column::DateColumnPartition&> date_columns;
   std::map<std::string, storage::column::PangoLineageColumnPartition&> pango_lineage_columns;

   uint32_t fill(
      const std::filesystem::path& input_file,
      const PangoLineageAliasLookup& alias_key,
      const silo::config::DatabaseConfig& database_config
   );

   [[nodiscard]] ColumnPartitionGroup getSubgroup(
      const std::vector<config::DatabaseMetadata>& fields
   ) const;
};

struct ColumnGroup {
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      for(auto& [name, store] : string_columns){
         archive & store;
      }
      for(auto& [name, store] : indexed_string_columns){
         archive & store;
      }
      for(auto& [name, store] : int_columns){
         archive & store;
      }
      for(auto& [name, store] : float_columns){
         archive & store;
      }
      for(auto& [name, store] : date_columns){
         archive & store;
      }
      for(auto& [name, store] : pango_lineage_columns){
         archive & store;
      }
      // clang-format on
   }

   std::vector<config::DatabaseMetadata> metadata;

   std::map<std::string, storage::column::StringColumn> string_columns;
   std::map<std::string, storage::column::IndexedStringColumn> indexed_string_columns;
   std::map<std::string, storage::column::IntColumn> int_columns;
   std::map<std::string, storage::column::FloatColumn> float_columns;
   std::map<std::string, storage::column::DateColumn> date_columns;
   std::map<std::string, storage::column::PangoLineageColumn> pango_lineage_columns;
};

}  // namespace silo::storage

#endif  // SILO_COLUMN_GROUP_H
