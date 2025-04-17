#include "silo/storage/column_group.h"

#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/date.h"
#include "silo/common/json_value_type.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/optional_bool.h"
#include "silo/common/panic.h"
#include "silo/config/database_config.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/column/column_type_visitor.h"

namespace silo::storage {

using silo::common::OptionalBool;
using silo::schema::ColumnType;

template <>
std::map<std::string, column::IndexedStringColumnPartition>& ColumnPartitionGroup::getColumns<
   column::IndexedStringColumnPartition>() {
   return indexed_string_columns;
}

template <>
std::map<std::string, column::StringColumnPartition>& ColumnPartitionGroup::getColumns<
   column::StringColumnPartition>() {
   return string_columns;
}

template <>
std::map<std::string, column::IntColumnPartition>& ColumnPartitionGroup::getColumns<
   column::IntColumnPartition>() {
   return int_columns;
}

template <>
std::map<std::string, column::BoolColumnPartition>& ColumnPartitionGroup::getColumns<
   column::BoolColumnPartition>() {
   return bool_columns;
}

template <>
std::map<std::string, column::FloatColumnPartition>& ColumnPartitionGroup::getColumns<
   column::FloatColumnPartition>() {
   return float_columns;
}

template <>
std::map<std::string, column::DateColumnPartition>& ColumnPartitionGroup::getColumns<
   column::DateColumnPartition>() {
   return date_columns;
}

template <>
std::map<std::string, column::SequenceColumnPartition<Nucleotide>>& ColumnPartitionGroup::
   getColumns<column::SequenceColumnPartition<Nucleotide>>() {
   return nuc_columns;
}

template <>
std::map<std::string, column::SequenceColumnPartition<AminoAcid>>& ColumnPartitionGroup::getColumns<
   column::SequenceColumnPartition<AminoAcid>>() {
   return aa_columns;
}

template <>
std::map<std::string, column::ZstdCompressedStringColumnPartition>& ColumnPartitionGroup::
   getColumns<column::ZstdCompressedStringColumnPartition>() {
   return zstd_compressed_string_columns;
}

template <>
const std::map<std::string, column::IndexedStringColumnPartition>& ColumnPartitionGroup::getColumns<
   column::IndexedStringColumnPartition>() const {
   return indexed_string_columns;
}

template <>
const std::map<std::string, column::StringColumnPartition>& ColumnPartitionGroup::getColumns<
   column::StringColumnPartition>() const {
   return string_columns;
}

template <>
const std::map<std::string, column::IntColumnPartition>& ColumnPartitionGroup::getColumns<
   column::IntColumnPartition>() const {
   return int_columns;
}

template <>
const std::map<std::string, column::BoolColumnPartition>& ColumnPartitionGroup::getColumns<
   column::BoolColumnPartition>() const {
   return bool_columns;
}

template <>
const std::map<std::string, column::FloatColumnPartition>& ColumnPartitionGroup::getColumns<
   column::FloatColumnPartition>() const {
   return float_columns;
}

template <>
const std::map<std::string, column::DateColumnPartition>& ColumnPartitionGroup::getColumns<
   column::DateColumnPartition>() const {
   return date_columns;
}

template <>
const std::map<std::string, column::SequenceColumnPartition<Nucleotide>>& ColumnPartitionGroup::
   getColumns<column::SequenceColumnPartition<Nucleotide>>() const {
   return nuc_columns;
}

template <>
const std::map<std::string, column::SequenceColumnPartition<AminoAcid>>& ColumnPartitionGroup::
   getColumns<column::SequenceColumnPartition<AminoAcid>>() const {
   return aa_columns;
}

template <>
const std::map<std::string, column::ZstdCompressedStringColumnPartition>& ColumnPartitionGroup::
   getColumns<column::ZstdCompressedStringColumnPartition>() const {
   return zstd_compressed_string_columns;
}

namespace {

template <typename SymbolType>
std::string getNdjsonSequenceStructName();

template <>
std::string getNdjsonSequenceStructName<Nucleotide>() {
   return "alignedNucleotideSequences";
}

template <>
std::string getNdjsonSequenceStructName<AminoAcid>() {
   return "alignedAminoAcidSequences";
}

template <typename SymbolType>
std::string getNdjsonInsertionStructName();

template <>
std::string getNdjsonInsertionStructName<Nucleotide>() {
   return "nucleotideInsertions";
}

template <>
std::string getNdjsonInsertionStructName<AminoAcid>() {
   return "aminoAcidInsertions";
}

template <typename SymbolType>
void insertToSequenceColumn(
   ColumnPartitionGroup& columns,
   const schema::ColumnIdentifier& column,
   const nlohmann::json& value
) {
   auto& sequence_column =
      columns.getColumns<column::SequenceColumnPartition<SymbolType>>().at(column.name);
   const nlohmann::json& sequence =
      value.at(getNdjsonSequenceStructName<SymbolType>()).at(column.name);
   const nlohmann::json& insertions =
      value.at(getNdjsonInsertionStructName<SymbolType>()).at(column.name);
   auto& read = sequence_column.appendNewSequenceRead();
   if (sequence.is_null()) {
      read.is_valid = false;
   } else {
      read.sequence = sequence.get<std::string>();
      read.offset = 0;
      read.is_valid = true;
   }
   for (auto& insertion : insertions) {
      sequence_column.appendInsertion(insertion.get<std::string>());
   }
}

class ColumnValueInserter {
  public:
   template <column::Column ColumnType>
   void operator()(
      ColumnPartitionGroup& columns,
      const schema::ColumnIdentifier& column,
      const nlohmann::json& value
   ) {
      auto column_value = value["metadata"][column.name];
      if (column_value.is_null()) {
         columns.getColumns<ColumnType>().at(column.name).insertNull();
      } else {
         columns.getColumns<ColumnType>().at(column.name).insert(column_value.get<std::string>());
      }
   }
};

template <>
void ColumnValueInserter::operator()<column::BoolColumnPartition>(
   ColumnPartitionGroup& columns,
   const schema::ColumnIdentifier& column,
   const nlohmann::json& value
) {
   auto column_value = value["metadata"][column.name];
   if (column_value.is_null()) {
      columns.getColumns<column::BoolColumnPartition>().at(column.name).insertNull();
   } else {
      columns.getColumns<column::BoolColumnPartition>()
         .at(column.name)
         .insert(column_value.get<bool>());
   }
}

template <>
void ColumnValueInserter::operator()<column::ZstdCompressedStringColumnPartition>(
   ColumnPartitionGroup& columns,
   const schema::ColumnIdentifier& column,
   const nlohmann::json& value
) {
   // TODO(#741) we prepend the unalignedSequence columns (which are using the type
   // ZstdCompressedStringColumnPartition) with 'unaligned_'. This should be cleaned up with a
   // refactor and breaking change of the current input format.
   auto column_value =
      value.at("unalignedNucleotideSequences")
         .at(column.name.substr(storage::UNALIGNED_NUCLEOTIDE_SEQUENCE_PREFIX.length()));
   if (column_value.is_null()) {
      columns.getColumns<column::ZstdCompressedStringColumnPartition>()
         .at(column.name)
         .insertNull();
   } else {
      columns.getColumns<column::ZstdCompressedStringColumnPartition>()
         .at(column.name)
         .insert(column_value.get<std::string>());
   }
}

template <>
void ColumnValueInserter::operator()<column::IntColumnPartition>(
   ColumnPartitionGroup& columns,
   const schema::ColumnIdentifier& column,
   const nlohmann::json& value
) {
   auto column_value = value.at("metadata").at(column.name);
   if (column_value.is_null()) {
      columns.getColumns<column::IntColumnPartition>().at(column.name).insertNull();
   } else {
      columns.getColumns<column::IntColumnPartition>()
         .at(column.name)
         .insert(column_value.get<int32_t>());
   }
}

template <>
void ColumnValueInserter::operator()<column::FloatColumnPartition>(
   ColumnPartitionGroup& columns,
   const schema::ColumnIdentifier& column,
   const nlohmann::json& value
) {
   auto column_value = value.at("metadata").at(column.name);
   if (column_value.is_null()) {
      columns.getColumns<column::FloatColumnPartition>().at(column.name).insertNull();
   } else {
      columns.getColumns<column::FloatColumnPartition>()
         .at(column.name)
         .insert(column_value.get<double>());
   }
}

template <>
void ColumnValueInserter::operator()<column::SequenceColumnPartition<AminoAcid>>(
   ColumnPartitionGroup& columns,
   const schema::ColumnIdentifier& column,
   const nlohmann::json& value
) {
   insertToSequenceColumn<AminoAcid>(columns, column, value);
}

template <>
void ColumnValueInserter::operator()<column::SequenceColumnPartition<Nucleotide>>(
   ColumnPartitionGroup& columns,
   const schema::ColumnIdentifier& column,
   const nlohmann::json& value
) {
   insertToSequenceColumn<Nucleotide>(columns, column, value);
}

}  // namespace

void ColumnPartitionGroup::addJsonValueToColumn(
   const schema::ColumnIdentifier& column,
   const nlohmann::json& value
) {
   column::visit(column.type, ColumnValueInserter{}, *this, column, value);
}

ColumnPartitionGroup ColumnPartitionGroup::getSubgroup(
   const std::vector<silo::schema::ColumnIdentifier>& fields
) const {
   ColumnPartitionGroup result;
   for (const auto& field : fields) {
      result.metadata.push_back({field.name, field.type});
   }
   auto columnCopier = []<column::Column ColumnType>(
                          const ColumnPartitionGroup& from,
                          const schema::ColumnIdentifier& column_identifier,
                          ColumnPartitionGroup& target
                       ) {
      target.getColumns<ColumnType>().emplace(
         column_identifier.name, from.getColumns<ColumnType>().at(column_identifier.name)
      );
   };

   for (const auto& item : fields) {
      column::visit(item.type, columnCopier, *this, item, result);
   }
   return result;
}

common::JsonValueType ColumnPartitionGroup::getValue(
   const std::string& column,
   uint32_t sequence_id
) const {
   if (string_columns.contains(column)) {
      return string_columns.at(column).lookupValue(
         string_columns.at(column).getValues().at(sequence_id)
      );
   }
   if (indexed_string_columns.contains(column)) {
      return indexed_string_columns.at(column).lookupValue(
         indexed_string_columns.at(column).getValues().at(sequence_id)
      );
   }
   if (date_columns.contains(column)) {
      return common::dateToString(date_columns.at(column).getValues().at(sequence_id));
   }
   if (bool_columns.contains(column)) {
      const OptionalBool value = bool_columns.at(column).getValues().at(sequence_id);
      if (value.isNull()) {
         return std::nullopt;
      }
      return value.value();
   }
   if (int_columns.contains(column)) {
      int32_t value = int_columns.at(column).getValues().at(sequence_id);
      if (value == column::IntColumnPartition::null()) {
         return std::nullopt;
      }
      return value;
   }
   if (float_columns.contains(column)) {
      double value = float_columns.at(column).getValues().at(sequence_id);
      if (value == std::nan("")) {
         return std::nullopt;
      }
      return value;
   }
   return std::nullopt;
}

}  // namespace silo::storage
