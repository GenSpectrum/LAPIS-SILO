#include "silo/query_engine/filter_expressions/insertion_contains.h"

#include <map>
#include <utility>

#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/database.h"
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/bitmap_producer.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/column/insertion_column.h"
#include "silo/storage/database_partition.h"

namespace silo {
class Database;
namespace query_engine::operators {
class Operator;
}  // namespace query_engine::operators
}  // namespace silo

namespace silo::query_engine::filter_expressions {

template <typename SymbolType>
InsertionContains<SymbolType>::InsertionContains(
   std::string column,
   std::optional<std::string> sequence_name,
   uint32_t position,
   std::string value
)
    : column_name(std::move(column)),
      sequence_name(std::move(sequence_name)),
      position(position),
      value(std::move(value)) {}

template <typename SymbolType>
std::string InsertionContains<SymbolType>::toString(const silo::Database& /*database*/) const {
   return column_name + " has insertion '" + value + "'";
}

template <typename SymbolType>
std::unique_ptr<silo::query_engine::operators::Operator> InsertionContains<SymbolType>::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      database_partition.columns.getInsertionColumns<SymbolType>().contains(column_name),
      "The insertion column '" + column_name + "' does not exist."
   )
   std::string validated_sequence_name;
   if (sequence_name.has_value()) {
      validated_sequence_name = sequence_name.value();
   } else {
      CHECK_SILO_QUERY(
         database.getDefaultSequenceName<SymbolType>().has_value(),
         "The database has no default " + std::string(SymbolType::SYMBOL_NAME_LOWER_CASE) +
            " sequence name"
      );
      validated_sequence_name = *database.getDefaultSequenceName<SymbolType>();
   }

   const storage::column::InsertionColumnPartition<SymbolType>& insertion_column =
      database_partition.columns.getInsertionColumns<SymbolType>().at(column_name);

   return std::make_unique<operators::BitmapProducer>(
      [&, validated_sequence_name]() {
         auto search_result = insertion_column.search(validated_sequence_name, position, value);
         return OperatorResult(std::move(*search_result));
      },
      database_partition.sequence_count
   );
}

namespace {

template <typename SymbolType>
std::regex buildValidInsertionSearchRegex() {
   // Build the following regex pattern: ^([symbols]|\.\*)*$
   std::stringstream regex_pattern_string;
   regex_pattern_string << "^([";
   for (const auto symbol : SymbolType::SYMBOLS) {
      regex_pattern_string << SymbolType::symbolToChar(symbol);
   }
   regex_pattern_string << "]|\\.\\*)*$";
   return std::regex(regex_pattern_string.str());
}

template <typename SymbolType>
const std::regex VALID_INSERTION_SEARCH_VALUE_REGEX = buildValidInsertionSearchRegex<SymbolType>();

template <typename SymbolType>
bool validateInsertionSearchValue(const std::string& value) {
   return std::regex_search(value, VALID_INSERTION_SEARCH_VALUE_REGEX<SymbolType>);
}

}  // namespace

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<InsertionContains<SymbolType>>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in an InsertionContains expression"
   )
   CHECK_SILO_QUERY(
      json["column"].is_string(),
      "The field 'column' in an InsertionContains expression needs to be a string"
   )
   CHECK_SILO_QUERY(
      json.contains("position"),
      "The field 'position' is required in an InsertionContains expression"
   )
   CHECK_SILO_QUERY(
      json["position"].is_number_unsigned() && (json["position"].get<uint32_t>() > 0),
      "The field 'position' in an InsertionContains expression needs to be a positive number (> 0)"
   )
   CHECK_SILO_QUERY(
      !json.contains("sequenceName") || json["sequenceName"].is_string(),
      "The optional field 'sequenceName' in an InsertionContains expression needs to be a string"
   )
   CHECK_SILO_QUERY(
      json.contains("value"), "The field 'value' is required in an InsertionContains expression"
   )
   CHECK_SILO_QUERY(
      json["value"].is_string() && !json["value"].is_null(),
      "The field 'value' in an InsertionContains expression needs to be a string"
   )
   const std::string& column_name = json["column"];
   std::optional<std::string> sequence_name;
   if (json.contains("sequenceName")) {
      sequence_name = json["sequenceName"].get<std::string>();
   }
   const uint32_t position = json["position"].get<uint32_t>();
   const std::string& value = json["value"].get<std::string>();
   CHECK_SILO_QUERY(
      !value.empty(),
      "The field 'value' in an InsertionContains expression must not be an empty string"
   )
   CHECK_SILO_QUERY(
      validateInsertionSearchValue<SymbolType>(value),
      "The field 'value' in the InsertionContains expression does not contain a valid regex "
      "pattern: \"" +
         value + "\". It must only consist of " + std::string(SymbolType::SYMBOL_NAME_LOWER_CASE) +
         " symbols and the regex symbol '.*'."
   )
   filter =
      std::make_unique<InsertionContains<SymbolType>>(column_name, sequence_name, position, value);
}

template void from_json<Nucleotide>(
   const nlohmann::json& json,
   std::unique_ptr<InsertionContains<Nucleotide>>& filter
);

template void from_json<AminoAcid>(
   const nlohmann::json& json,
   std::unique_ptr<InsertionContains<AminoAcid>>& filter
);

template class InsertionContains<Nucleotide>;
template class InsertionContains<AminoAcid>;

}  // namespace silo::query_engine::filter_expressions