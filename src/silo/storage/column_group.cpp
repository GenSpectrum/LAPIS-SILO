#include "silo/storage/column_group.h"

#include <cmath>
#include <cstdlib>
#include <expected>
#include <stdexcept>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>

#include "evobench/evobench.hpp"
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

#define RAISE_STRING_ERROR_WITH_CONTEXT(error, value, ...)                                  \
   if (error) {                                                                             \
      simdjson::ondemand::raw_json_string line_context;                                     \
      auto error_when_getting_line_context = value.get_raw_json_string().get(line_context); \
      if (error_when_getting_line_context) {                                                \
         return std::unexpected{fmt::format(__VA_ARGS__, simdjson::error_message(error))};  \
      } else {                                                                              \
         return std::unexpected{fmt::format(                                                \
            "{}. Current line: {}",                                                         \
            fmt::format(__VA_ARGS__, simdjson::error_message(error)),                       \
            line_context.raw()                                                              \
         )};                                                                                \
      }                                                                                     \
   }

template <typename SymbolType>
std::expected<void, std::string> insertToSequenceColumn(
   ColumnPartitionGroup& columns,
   const schema::ColumnIdentifier& column,
   simdjson::ondemand::value& value
) {
   auto& sequence_column =
      columns.getColumns<column::SequenceColumnPartition<SymbolType>>().at(column.name);
   auto& read = sequence_column.appendNewSequenceRead();
   bool is_null;
   auto error = value.is_null().get(is_null);
   RAISE_STRING_ERROR_WITH_CONTEXT(
      error, value, "When checking column field '{}' for null got error: {}", column.name
   );
   if (is_null) {
      read.is_valid = false;
      return {};
   }
   std::string_view sequence;
   error = value["sequence"].get(sequence);
   RAISE_STRING_ERROR_WITH_CONTEXT(
      error, value, "When getting field 'sequence' in column field '{}' got error: {}", column.name
   );
   // TODO(#877) std::optional<uint32_t> offset = value["offset"].get<std::optional<uint32_t>>();
   std::vector<std::string> insertions;
   error = value["insertions"].get(insertions);
   RAISE_STRING_ERROR_WITH_CONTEXT(
      error,
      value,
      "When getting field 'insertions' in column field '{}' got error: {}",
      column.name
   );
   read.sequence = sequence;
   // TODO(#877) read.offset = offset.value_or(0);
   read.offset = 0;
   read.is_valid = true;
   for (auto& insertion : insertions) {
      sequence_column.appendInsertion(insertion);
   }
   return {};
}

class ColumnValueInserter {
  public:
   template <column::Column ColumnType>
   std::expected<void, std::string> operator()(
      ColumnPartitionGroup& columns,
      const schema::ColumnIdentifier& column,
      simdjson::ondemand::value& value
   ) {
      bool is_null;
      auto error = value.is_null().get(is_null);
      RAISE_STRING_ERROR_WITH_CONTEXT(
         error, value, "When checking column field '{}' for null got error: {}", column.name
      );
      if (is_null) {
         columns.getColumns<ColumnType>().at(column.name).insertNull();
      } else {
         std::string_view column_value;
         error = value.get(column_value);
         RAISE_STRING_ERROR_WITH_CONTEXT(
            error,
            value,
            "When trying to get string value of column '{}' got error: {}",
            column.name
         );
         columns.getColumns<ColumnType>().at(column.name).insert(column_value);
      }
      return {};
   }
};

template <>
std::expected<void, std::string> ColumnValueInserter::operator()<column::BoolColumnPartition>(
   ColumnPartitionGroup& columns,
   const schema::ColumnIdentifier& column,
   simdjson::ondemand::value& value
) {
   bool is_null;
   auto error = value.is_null().get(is_null);
   RAISE_STRING_ERROR_WITH_CONTEXT(
      error, value, "When checking column field '{}' for null got error: {}", column.name
   );
   if (is_null) {
      columns.getColumns<column::BoolColumnPartition>().at(column.name).insertNull();
   } else {
      bool column_value;
      error = value.get(column_value);
      RAISE_STRING_ERROR_WITH_CONTEXT(
         error, value, "When trying to get bool value of column '{}' got error: {}", column.name
      );
      columns.getColumns<column::BoolColumnPartition>().at(column.name).insert(column_value);
   }
   return {};
}

template <>
std::expected<void, std::string> ColumnValueInserter::operator()<column::IntColumnPartition>(
   ColumnPartitionGroup& columns,
   const schema::ColumnIdentifier& column,
   simdjson::ondemand::value& value
) {
   bool is_null;
   auto error = value.is_null().get(is_null);
   RAISE_STRING_ERROR_WITH_CONTEXT(
      error, value, "When checking column field '{}' for null got error: {}", column.name
   );
   if (is_null) {
      columns.getColumns<column::IntColumnPartition>().at(column.name).insertNull();
   } else {
      int32_t column_value;
      error = value.get(column_value);
      RAISE_STRING_ERROR_WITH_CONTEXT(
         error, value, "When trying to get int32_t value of column '{}' got error: {}", column.name
      );
      columns.getColumns<column::IntColumnPartition>().at(column.name).insert(column_value);
   }
   return {};
}

template <>
std::expected<void, std::string> ColumnValueInserter::operator()<column::FloatColumnPartition>(
   ColumnPartitionGroup& columns,
   const schema::ColumnIdentifier& column,
   simdjson::ondemand::value& value
) {
   bool is_null;
   auto error = value.is_null().get(is_null);
   RAISE_STRING_ERROR_WITH_CONTEXT(
      error, value, "When checking column field '{}' for null got error: {}", column.name
   );
   if (is_null) {
      columns.getColumns<column::FloatColumnPartition>().at(column.name).insertNull();
   } else {
      double column_value;
      error = value.get(column_value);
      RAISE_STRING_ERROR_WITH_CONTEXT(
         error, value, "When trying to get double value of column '{}' got error: {}", column.name
      );
      columns.getColumns<column::FloatColumnPartition>().at(column.name).insert(column_value);
   }
   return {};
}

template <>
std::expected<void, std::string> ColumnValueInserter::operator(
)<column::SequenceColumnPartition<AminoAcid>>(
   ColumnPartitionGroup& columns,
   const schema::ColumnIdentifier& column,
   simdjson::ondemand::value& value
) {
   return insertToSequenceColumn<AminoAcid>(columns, column, value);
}

template <>
std::expected<void, std::string> ColumnValueInserter::operator(
)<column::SequenceColumnPartition<Nucleotide>>(
   ColumnPartitionGroup& columns,
   const schema::ColumnIdentifier& column,
   simdjson::ondemand::value& value
) {
   return insertToSequenceColumn<Nucleotide>(columns, column, value);
}

}  // namespace

std::expected<void, std::string> ColumnPartitionGroup::addJsonValueToColumn(
   const schema::ColumnIdentifier& column,
   simdjson::ondemand::value& value
) {
   EVOBENCH_SCOPE_EVERY(1000, "ColumnPartitionGroup", "addJsonValueToColumn");
   return column::visit(column.type, ColumnValueInserter{}, *this, column, value);
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
      auto value =
         string_columns.at(column).lookupValue(string_columns.at(column).getValues().at(sequence_id)
         );
      if (value.empty()) {
         return std::nullopt;
      }
      return value;
   }
   if (indexed_string_columns.contains(column)) {
      auto value = indexed_string_columns.at(column).lookupValue(
         indexed_string_columns.at(column).getValues().at(sequence_id)
      );
      if (value.empty()) {
         return std::nullopt;
      }
      return std::string{value};
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
   SILO_PANIC("Called getValue for the column {} that does not exist in the schema", column);
}

}  // namespace silo::storage
