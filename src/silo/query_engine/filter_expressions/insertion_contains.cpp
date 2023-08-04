#include "silo/query_engine/filter_expressions/insertion_contains.h"

#include <map>
#include <utility>

#include <nlohmann/json.hpp>

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

InsertionContains::InsertionContains(std::string column, uint32_t position, std::string value)
    : column_name(std::move(column)),
      position(position),
      value(std::move(value)) {}

std::string InsertionContains::toString(const silo::Database& /*database*/) const {
   return column_name + " has insertion '" + value + "'";
}

std::unique_ptr<silo::query_engine::operators::Operator> InsertionContains::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      database_partition.columns.insertion_columns.contains(column_name),
      "The insertion column '" + column_name + "' does not exist."
   )

   const storage::column::InsertionColumnPartition& insertion_column =
      database_partition.columns.insertion_columns.at(column_name);

   return std::make_unique<operators::BitmapProducer>(
      [&]() {
         auto search_result = insertion_column.search(position, value);
         return OperatorResult(std::move(*search_result));
      },
      database_partition.sequence_count
   );
}

namespace {

std::regex buildValidInsertionSearchRegex() {
   // Build the following regex pattern: ^([nuc-symbols]|\.\*)*$
   std::stringstream regex_pattern_string;
   regex_pattern_string << "^([";
   for (const auto nuc : NUC_SYMBOLS) {
      regex_pattern_string << nucleotideSymbolToChar(nuc);
   }
   regex_pattern_string << "]|\\.\\*)*$";
   return std::regex(regex_pattern_string.str());
}

const std::regex VALID_INSERTION_SEARCH_VALUE_REGEX = buildValidInsertionSearchRegex();

bool validateInsertionSearchValue(const std::string& value) {
   return std::regex_search(value, VALID_INSERTION_SEARCH_VALUE_REGEX);
}

}  // namespace

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<InsertionContains>& filter) {
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
      json.contains("value"), "The field 'value' is required in an InsertionContains expression"
   )
   CHECK_SILO_QUERY(
      json["value"].is_string() && !json["value"].is_null(),
      "The field 'value' in an InsertionContains expression needs to be a string"
   )
   const std::string& column_name = json["column"];
   const uint32_t position = json["position"];
   const std::string& value = json["value"].get<std::string>();
   CHECK_SILO_QUERY(
      !value.empty(),
      "The field 'value' in an InsertionContains expression must not be an empty string"
   )
   CHECK_SILO_QUERY(
      validateInsertionSearchValue(value),
      "The field 'value' in the InsertionContains expression does not contain a valid regex "
      "pattern: \"" +
         value + "\". It must only consist of nucleotide symbols and the regex symbol '.*'."
   )
   filter = std::make_unique<InsertionContains>(column_name, position, value);
}

}  // namespace silo::query_engine::filter_expressions