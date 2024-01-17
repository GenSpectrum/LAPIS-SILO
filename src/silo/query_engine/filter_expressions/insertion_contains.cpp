#include "silo/query_engine/filter_expressions/insertion_contains.h"

#include <ostream>
#include <regex>
#include <sstream>
#include <utility>

#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>
#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/database.h"
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/bitmap_producer.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/operators/union.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/column/insertion_column.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

template <typename SymbolType>
InsertionContains<SymbolType>::InsertionContains(
   std::vector<std::string>&& column_names,
   std::optional<std::string> sequence_name,
   uint32_t position,
   std::string value
)
    : column_names(std::move(column_names)),
      sequence_name(std::move(sequence_name)),
      position(position),
      value(std::move(value)) {}

std::string getColumnNamesString(
   const std::vector<std::string>& column_names,
   const std::string& symbol_name
) {
   if (column_names.empty()) {
      return "in all available " + symbol_name + " columns";
   }
   if (column_names.size() == 1) {
      return "in the " + symbol_name + " column " + column_names.at(0);
   }
   return "in the " + symbol_name + " columns [" + boost::algorithm::join(column_names, ",") + "]";
}

template <typename SymbolType>
std::string InsertionContains<SymbolType>::toString(const silo::Database& /*database*/) const {
   const std::string symbol_name = std::string(SymbolType::SYMBOL_NAME);
   const std::string sequence_string = sequence_name
                                          ? "The sequence '" + sequence_name.value() + "'"
                                          : "The default " + symbol_name + " sequence ";

   const std::string columns_string = getColumnNamesString(column_names, symbol_name);

   return sequence_string + columns_string + " has insertion '" + value + "'";
}

template <typename SymbolType>
std::unique_ptr<silo::query_engine::operators::Operator> InsertionContains<SymbolType>::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   SPDLOG_TRACE(
      "In InsertionContains<SymbolType>::compile with column names: " +
      boost::join(column_names, ",")
   );
   for (const std::string& column_name : column_names) {
      CHECK_SILO_QUERY(
         database_partition.columns.getInsertionColumns<SymbolType>().contains(column_name),
         "The insertion column '" + column_name + "' does not exist."
      )
   }
   if (database_partition.columns.getInsertionColumns<SymbolType>().empty()) {
      return std::make_unique<operators::Empty>(database_partition.sequence_count);
   }

   std::string validated_sequence_name;
   if (sequence_name.has_value()) {
      validated_sequence_name = sequence_name.value();
   } else {
      CHECK_SILO_QUERY(
         database.getDefaultSequenceName<SymbolType>().has_value(),
         "The database has no default " + std::string(SymbolType::SYMBOL_NAME_LOWER_CASE) +
            " sequence name"
      )
      // NOLINTNEXTLINE(bugprone-unchecked-optional-access) -- the previous statement checks it
      validated_sequence_name = *database.getDefaultSequenceName<SymbolType>();
   }

   std::vector<std::unique_ptr<operators::Operator>> column_operators;

   for (const auto& [column_name, insertion_column] :
        database_partition.columns.getInsertionColumns<SymbolType>()) {
      const storage::column::InsertionColumnPartition<SymbolType>& the_insertion_column =
         insertion_column;
      if(column_names.empty() || std::find(column_names.begin(), column_names.end(), column_name) != column_names.end()){
         if (the_insertion_column.getInsertionIndexes().contains(validated_sequence_name)) {
            column_operators.emplace_back(std::make_unique<operators::BitmapProducer>(
               [&, validated_sequence_name]() {
                  auto search_result =
                     the_insertion_column.search(validated_sequence_name, position, value);
                  return OperatorResult(std::move(*search_result));
               },
               database_partition.sequence_count
            ));
         }
      }
   }

   if (column_operators.empty()) {
      return std::make_unique<operators::Empty>(database_partition.sequence_count);
   }
   if (column_operators.size() == 1) {
      return std::move(column_operators.at(0));
   }
   return std::make_unique<operators::Union>(
      std::move(column_operators), database_partition.sequence_count
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
      !json.contains("column") || (json["column"].is_string() || json["column"].is_array()),
      "The InsertionsContains filter can have the field column of type string or an array of "
      "strings, but no other type"
   )
   std::vector<std::string> column_names;
   if (json.contains("column") && json.at("column").is_array()) {
      for (const auto& child : json["column"]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "The field column of the InsertionsContains filter must have type string or an "
            "array, if present. Found:" +
               child.dump()
         )
         column_names.emplace_back(child.get<std::string>());
      }
   } else if (json.contains("column") && json["column"].is_string()) {
      column_names.emplace_back(json["column"].get<std::string>());
   }

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
   filter = std::make_unique<InsertionContains<SymbolType>>(
      std::move(column_names), sequence_name, position, value
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
template void from_json<Nucleotide>(
   const nlohmann::json& json,
   std::unique_ptr<InsertionContains<Nucleotide>>& filter
);

// NOLINTNEXTLINE(readability-identifier-naming)
template void from_json<AminoAcid>(
   const nlohmann::json& json,
   std::unique_ptr<InsertionContains<AminoAcid>>& filter
);

template class InsertionContains<Nucleotide>;
template class InsertionContains<AminoAcid>;

}  // namespace silo::query_engine::filter_expressions