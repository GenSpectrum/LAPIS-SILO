#pragma once

#include <cstdint>
#include <expected>
#include <map>
#include <string>
#include <vector>

#include <simdjson.h>
#include <boost/serialization/access.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/bool_column.h"
#include "silo/storage/column/date32_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/column/zstd_compressed_string_column.h"

namespace silo::storage {

class ColumnGroup {
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
      for(auto& [name, store] : date32_columns){
         archive & store;
      }
      for(auto& [name, store] : nuc_columns){
         archive & store;
      }
      for(auto& [name, store] : aa_columns){
         archive & store;
      }
      for(auto& [name, store] : zstd_compressed_string_columns){
         archive & store;
      }
      // clang-format on
   }

  public:
   std::vector<silo::schema::ColumnIdentifier> metadata;

   std::map<std::string, column::StringColumn> string_columns;
   std::map<std::string, column::IndexedStringColumn> indexed_string_columns;
   std::map<std::string, column::BoolColumn> bool_columns;
   std::map<std::string, column::IntColumn> int_columns;
   std::map<std::string, column::FloatColumn> float_columns;
   std::map<std::string, column::Date32Column> date32_columns;
   std::map<std::string, column::SequenceColumn<Nucleotide>> nuc_columns;
   std::map<std::string, column::SequenceColumn<AminoAcid>> aa_columns;
   std::map<std::string, column::ZstdCompressedStringColumn>
      zstd_compressed_string_columns;

   std::expected<void, std::string> addJsonValueToColumn(
      const schema::ColumnIdentifier& column_identifier,
      simdjson::ondemand::value& value
   );

   template <column::Column ColumnType>
   std::map<std::string, ColumnType>& getColumns();

   template <column::Column ColumnType>
   [[nodiscard]] const std::map<std::string, ColumnType>& getColumns() const;
};

}  // namespace silo::storage
