#ifndef SILO_COLUMN_GROUP_H
#define SILO_COLUMN_GROUP_H

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "silo/config/database_config.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/insertion_column.h"
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
      for(auto& [name, store] : nuc_insertion_columns){
         archive & store;
      }
      for(auto& [name, store] : aa_insertion_columns){
         archive & store;
      }
      // clang-format on
   }

  public:
   std::vector<ColumnMetadata> metadata;

   std::map<std::string, storage::column::StringColumnPartition&> string_columns;
   std::map<std::string, storage::column::IndexedStringColumnPartition&> indexed_string_columns;
   std::map<std::string, storage::column::IntColumnPartition&> int_columns;
   std::map<std::string, storage::column::FloatColumnPartition&> float_columns;
   std::map<std::string, storage::column::DateColumnPartition&> date_columns;
   std::map<std::string, storage::column::PangoLineageColumnPartition&> pango_lineage_columns;
   std::map<std::string, storage::column::InsertionColumnPartition<Nucleotide>&>
      nuc_insertion_columns;
   std::map<std::string, storage::column::InsertionColumnPartition<AminoAcid>&>
      aa_insertion_columns;

   uint32_t fill(
      const std::filesystem::path& input_file,
      const silo::config::DatabaseConfig& database_config
   );

   [[nodiscard]] ColumnPartitionGroup getSubgroup(
      const std::vector<silo::storage::ColumnMetadata>& fields
   ) const;

   std::optional<std::variant<std::string, int32_t, double>> getValue(
      const std::string& column,
      uint32_t sequence_id
   ) const;

   template <typename SymbolType>
   const std::map<std::string, storage::column::InsertionColumnPartition<SymbolType>&>&
   getInsertionColumns() const;
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
      for(auto& [_, store] : nuc_insertion_columns){
         archive & store;
      }
      for(auto& [_, store] : aa_insertion_columns){
         archive & store;
      }
      // clang-format on
   }

  public:
   std::vector<ColumnMetadata> metadata;

   std::map<std::string, storage::column::StringColumn> string_columns;
   std::map<std::string, storage::column::IndexedStringColumn> indexed_string_columns;
   std::map<std::string, storage::column::IntColumn> int_columns;
   std::map<std::string, storage::column::FloatColumn> float_columns;
   std::map<std::string, storage::column::DateColumn> date_columns;
   std::map<std::string, storage::column::PangoLineageColumn> pango_lineage_columns;
   std::map<std::string, storage::column::InsertionColumn<Nucleotide>> nuc_insertion_columns;
   std::map<std::string, storage::column::InsertionColumn<AminoAcid>> aa_insertion_columns;
};

}  // namespace silo::storage

#endif  // SILO_COLUMN_GROUP_H
