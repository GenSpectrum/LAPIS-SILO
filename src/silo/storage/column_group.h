#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <boost/serialization/access.hpp>
#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/json_value_type.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/config/database_config.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/bool_column.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/column/zstd_compressed_string_column.h"

namespace silo::storage {

// TODO(#741) we prepend the unalignedSequence columns (which are using the type
// ZstdCompressedStringColumnPartition) with 'unaligned_'. This should be cleaned up with a
// refactor and breaking change of the current input format.
static const std::string UNALIGNED_NUCLEOTIDE_SEQUENCE_PREFIX = "unaligned_";

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

   std::map<std::string, column::StringColumnPartition> string_columns;
   std::map<std::string, column::IndexedStringColumnPartition> indexed_string_columns;
   std::map<std::string, column::BoolColumnPartition> bool_columns;
   std::map<std::string, column::IntColumnPartition> int_columns;
   std::map<std::string, column::FloatColumnPartition> float_columns;
   std::map<std::string, column::DateColumnPartition> date_columns;
   std::map<std::string, column::SequenceColumnPartition<Nucleotide>> nuc_columns;
   std::map<std::string, column::SequenceColumnPartition<AminoAcid>> aa_columns;
   std::map<std::string, column::ZstdCompressedStringColumnPartition>
      zstd_compressed_string_columns;

   void addJsonValueToColumn(
      const schema::ColumnIdentifier& column_identifier,
      const nlohmann::json& value
   );

   [[nodiscard]] ColumnPartitionGroup getSubgroup(
      const std::vector<schema::ColumnIdentifier>& fields
   ) const;

   [[nodiscard]] common::JsonValueType getValue(const std::string& column, uint32_t sequence_id)
      const;

   template <column::Column ColumnType>
   std::map<std::string, ColumnType>& getColumns();

   template <column::Column ColumnType>
   const std::map<std::string, ColumnType>& getColumns() const;
};

}  // namespace silo::storage
