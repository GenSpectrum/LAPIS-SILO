#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>

#include "rhydb/common/aa_symbols.h"
#include "rhydb/common/nucleotide_symbols.h"
#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/column/bool_column.h"
#include "rhydb/storage/column/date32_column.h"
#include "rhydb/storage/column/dictionary_encoded_column.h"
#include "rhydb/storage/column/float_column.h"
#include "rhydb/storage/column/int_column.h"
#include "rhydb/storage/column/sequence_column.h"
#include "rhydb/storage/column/string_column.h"
#include "rhydb/storage/column/zstd_compressed_string_column.h"

namespace rhydb::storage {

class ColumnGroup {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /*version*/) {
      // clang-format off
      for(auto& [name, store] : string_columns){
         archive & store;
      }
      for(auto& [name, store] : dictionary_encoded_columns){
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
   std::vector<rhydb::schema::ColumnIdentifier> metadata;

   std::map<std::string, column::StringColumn> string_columns;
   std::map<std::string, column::DictionaryEncodedColumn> dictionary_encoded_columns;
   std::map<std::string, column::BoolColumn> bool_columns;
   std::map<std::string, column::IntColumn> int_columns;
   std::map<std::string, column::FloatColumn> float_columns;
   std::map<std::string, column::Date32Column> date32_columns;
   std::map<std::string, column::SequenceColumn<Nucleotide>> nuc_columns;
   std::map<std::string, column::SequenceColumn<AminoAcid>> aa_columns;
   std::map<std::string, column::ZstdCompressedStringColumn> zstd_compressed_string_columns;

   template <column::Column ColumnType>
   std::map<std::string, ColumnType>& getColumns();

   template <column::Column ColumnType>
   [[nodiscard]] const std::map<std::string, ColumnType>& getColumns() const;
};

}  // namespace rhydb::storage
