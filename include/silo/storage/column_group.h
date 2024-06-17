#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "silo/common/aa_symbols.h"
#include "silo/common/json_value_type.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/storage/column/bool_column.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/pango_lineage_column.h"
#include "silo/storage/column/string_column.h"

namespace boost::serialization {
class access;
}  // namespace boost::serialization

namespace duckdb {
class Connection;
class Value;
}  // namespace duckdb

namespace silo::config {
class DatabaseConfig;
enum class ColumnType;
}  // namespace silo::config

namespace silo::storage {

struct ColumnMetadata {
   std::string name;
   silo::config::ColumnType type;
};

class ColumnPartitionGroup {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      for(auto& [name, store] : string_columns){
         archive & store;
      }
      for(auto& [name, store] : indexed_string_columns){
         archive & store;
      }
      for(auto& [name, store] : bool_columns){
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

  public:
   std::vector<ColumnMetadata> metadata;

   std::map<std::string, storage::column::StringColumnPartition&> string_columns;
   std::map<std::string, storage::column::IndexedStringColumnPartition&> indexed_string_columns;
   std::map<std::string, storage::column::BoolColumnPartition&> bool_columns;
   std::map<std::string, storage::column::IntColumnPartition&> int_columns;
   std::map<std::string, storage::column::FloatColumnPartition&> float_columns;
   std::map<std::string, storage::column::DateColumnPartition&> date_columns;
   std::map<std::string, storage::column::PangoLineageColumnPartition&> pango_lineage_columns;

   void addValueToColumn(
      const std::string& column_name,
      config::ColumnType column_type,
      const duckdb::Value& value
   );

   void addNullToColumn(const std::string& column_name, config::ColumnType column_type);

   void reserveSpaceInColumn(
      const std::string& column_name,
      config::ColumnType column_type,
      size_t row_count
   );

   [[nodiscard]] ColumnPartitionGroup getSubgroup(
      const std::vector<silo::storage::ColumnMetadata>& fields
   ) const;

   [[nodiscard]] common::JsonValueType getValue(const std::string& column, uint32_t sequence_id)
      const;
};

class ColumnGroup {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      for(auto& [_, store] : string_columns){
         archive & store;
      }
      for(auto& [_, store] : indexed_string_columns){
         archive & store;
      }
      for(auto& [_, store] : bool_columns){
         archive & store;
      }
      for(auto& [_, store] : int_columns){
         archive & store;
      }
      for(auto& [_, store] : float_columns){
         archive & store;
      }
      for(auto& [_, store] : date_columns){
         archive & store;
      }
      for(auto& [_, store] : pango_lineage_columns){
         archive & store;
      }
      // clang-format on
   }

  public:
   std::vector<ColumnMetadata> metadata;

   std::map<std::string, storage::column::StringColumn> string_columns;
   std::map<std::string, storage::column::IndexedStringColumn> indexed_string_columns;
   std::map<std::string, storage::column::BoolColumn> bool_columns;
   std::map<std::string, storage::column::IntColumn> int_columns;
   std::map<std::string, storage::column::FloatColumn> float_columns;
   std::map<std::string, storage::column::DateColumn> date_columns;
   std::map<std::string, storage::column::PangoLineageColumn> pango_lineage_columns;
};

}  // namespace silo::storage
