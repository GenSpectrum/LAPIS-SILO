#include "silo/storage/column_group.h"

#include <cmath>
#include <cstdlib>
#include <expected>
#include <string>
#include <variant>
#include <vector>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>

#include "evobench/evobench.hpp"
#include "silo/common/aa_symbols.h"
#include "silo/common/base64.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/storage/column/column_type_visitor.h"

namespace silo::storage {

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
      }                                                                                     \
      return std::unexpected{fmt::format(                                                   \
         "{}. Current line: {}",                                                            \
         fmt::format(__VA_ARGS__, simdjson::error_message(error)),                          \
         line_context.raw()                                                                 \
      )};                                                                                   \
   }

struct InputSequence {
   std::variant<std::string_view, std::string> sequence;

   [[nodiscard]] std::string_view getView() const {
      if (std::holds_alternative<std::string_view>(sequence)) {
         return std::get<std::string_view>(sequence);
      }
      return std::get<std::string>(sequence);
   }
};

template <typename SymbolType>
std::expected<InputSequence, std::string> getSequenceFromJsonLine(
   simdjson::ondemand::value& value,
   std::string_view column_name,
   column::SequenceColumnPartition<SymbolType>& sequence_column
) {
   // Determine sequence: try 'sequenceCompressed' (base64-encoded zstd-compressed) first,
   // then fall back to plain 'sequence'.
   InputSequence input_sequence;
   auto compressed_field = value["sequenceCompressed"];
   if (!compressed_field.error()) {
      std::string_view compressed_base64;
      auto error = compressed_field.get(compressed_base64);
      RAISE_STRING_ERROR_WITH_CONTEXT(
         error, value, "error getting field 'sequenceCompressed' in object: {}"
      );
      auto decoded = decodeBase64(compressed_base64);
      if (!decoded.has_value()) {
         return std::unexpected{fmt::format(
            "invalid base64 in 'sequenceCompressed' for column '{}': {}. base64 encoded data "
            "length: {}",
            column_name,
            decoded.error(),
            compressed_base64.size()
         )};
      }
      try {
         std::string buffer;
         sequence_column.compressed_input_decompressor.decompress(*decoded, buffer);
         input_sequence.sequence = std::move(buffer);
      } catch (const std::runtime_error& ex) {
         return std::unexpected{
            fmt::format("failed to decompress 'sequenceCompressed': {}", ex.what())
         };
      }
   } else {
      std::string_view sequence_view;
      auto error = value["sequence"].get(sequence_view);
      RAISE_STRING_ERROR_WITH_CONTEXT(error, value, "error getting field 'sequence' in object: {}");
      input_sequence.sequence = sequence_view;
   }
   return input_sequence;
}

template <typename SymbolType>
std::expected<void, std::string> insertToSequenceColumn(
   ColumnPartitionGroup& columns,
   const schema::ColumnIdentifier& column,
   simdjson::ondemand::value& value
) {
   auto& sequence_column =
      columns.getColumns<column::SequenceColumnPartition<SymbolType>>().at(column.name);
   bool is_null;
   auto error = value.is_null().get(is_null);
   RAISE_STRING_ERROR_WITH_CONTEXT(error, value, "error checking value for null: {}");
   if (is_null) {
      sequence_column.appendNull();
      return {};
   }
   auto input_sequence = getSequenceFromJsonLine(value, column.name, sequence_column);
   if (!input_sequence.has_value()) {
      return std::unexpected{input_sequence.error()};
   }
   const std::string_view sequence = input_sequence.value().getView();
   uint32_t offset = 0;
   auto offset_in_file = value["offset"];
   if (!offset_in_file.error()) {
      error = offset_in_file.get<uint32_t>().get(offset);
      RAISE_STRING_ERROR_WITH_CONTEXT(error, value, "error getting field 'offset' in object: {}");
   }
   std::vector<std::string> insertions;
   error = value["insertions"].get(insertions);
   RAISE_STRING_ERROR_WITH_CONTEXT(error, value, "error getting field 'insertions' in object: {}");
   sequence_column.append(sequence, offset, insertions);
   return {};
}

class ColumnValueInserter {
  private:
   template <typename T>
   static constexpr std::string_view valueTypeName() {
      if constexpr (std::is_same_v<T, bool>) {
         return "boolean";
      } else if constexpr (std::is_same_v<T, int32_t>) {
         return "int32";
      } else if constexpr (std::is_same_v<T, double>) {
         return "double";
      } else if constexpr (std::is_same_v<T, std::string_view>) {
         return "string";
      } else {
         static_assert(!std::is_same_v<T, T>, "Unhandled value_type in valueTypeName");
      }
   }

  public:
   template <column::Column ColumnType>
   std::expected<void, std::string> operator()(
      ColumnPartitionGroup& columns,
      const schema::ColumnIdentifier& column,
      simdjson::ondemand::value& value
   ) {
      bool is_null;
      auto error = value.is_null().get(is_null);
      RAISE_STRING_ERROR_WITH_CONTEXT(error, value, "error checking value for null: {}");
      if (is_null) {
         columns.getColumns<ColumnType>().at(column.name).insertNull();
      } else {
         typename ColumnType::value_type column_value;
         error = value.get(column_value);
         RAISE_STRING_ERROR_WITH_CONTEXT(
            error,
            value,
            "error getting value as {}: {}. {}",
            valueTypeName<typename ColumnType::value_type>(),
            value.raw_json_token()
         );
         return columns.getColumns<ColumnType>().at(column.name).insert(column_value);
      }
      return {};
   }
};

template <>
std::expected<void, std::string> ColumnValueInserter::operator()<column::DateColumnPartition>(
   ColumnPartitionGroup& columns,
   const schema::ColumnIdentifier& column,
   simdjson::ondemand::value& value
) {
   bool is_null;
   auto error = value.is_null().get(is_null);
   RAISE_STRING_ERROR_WITH_CONTEXT(error, value, "error checking value for null: {}");
   if (is_null) {
      columns.getColumns<column::DateColumnPartition>().at(column.name).insertNull();
   } else {
      std::string_view column_value;
      error = value.get(column_value);
      RAISE_STRING_ERROR_WITH_CONTEXT(
         error, value, "error getting value as string: {}. {}", value.raw_json_token()
      );
      return columns.getColumns<column::DateColumnPartition>().at(column.name).insert(column_value);
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
   auto success = column::visit(column.type, ColumnValueInserter{}, *this, column, value);
   if (!success.has_value()) {
      return std::unexpected(
         fmt::format("error inserting into column '{}': {}", column.name, success.error())
      );
   }
   return success;
}

}  // namespace silo::storage
