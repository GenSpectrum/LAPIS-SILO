#include "silo/query_engine/filter/expressions/string_search.h"

#include <optional>
#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/common/german_string.h"
#include "silo/common/panic.h"
#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/bitmap_producer.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

StringSearch::StringSearch(std::string column_name, std::unique_ptr<re2::RE2> search_expression)
    : column_name(std::move(column_name)),
      search_expression(std::move(search_expression)) {}

std::string StringSearch::toString() const {
   return fmt::format("column {} regex_matches \"{}\"", column_name, search_expression->pattern());
}

namespace {
template <typename GenericStringColumn>
std::unique_ptr<silo::query_engine::filter::operators::Operator> createMatchingBitmap(
   const GenericStringColumn& string_column,
   const RE2& search_expression,
   size_t row_count
) {
   return std::make_unique<operators::BitmapProducer>(
      [&, row_count]() {
         roaring::Roaring result_bitmap;
         for (size_t row_idx = 0; row_idx < row_count; ++row_idx) {
            std::string full_string = string_column.getValueString(row_idx);
            if (re2::RE2::PartialMatch(full_string, search_expression)) {
               result_bitmap.add(row_idx);
            }
         }
         return CopyOnWriteBitmap(std::move(result_bitmap));
      },
      row_count
   );
}

}  // namespace

std::unique_ptr<silo::query_engine::filter::operators::Operator> StringSearch::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      table_partition.columns.string_columns.contains(column_name) ||
         table_partition.columns.indexed_string_columns.contains(column_name),
      "The database does not contain the string column '{}'",
      column_name
   )

   if (table_partition.columns.indexed_string_columns.contains(column_name)) {
      const auto& string_column = table_partition.columns.indexed_string_columns.at(column_name);
      return createMatchingBitmap(
         string_column, *search_expression, table_partition.sequence_count
      );
   }
   SILO_ASSERT(table_partition.columns.string_columns.contains(column_name));
   const auto& string_column = table_partition.columns.string_columns.at(column_name);
   return createMatchingBitmap(string_column, *search_expression, table_partition.sequence_count);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<StringSearch>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in an StringSearch expression"
   )
   CHECK_SILO_QUERY(
      json["column"].is_string(),
      "The field 'column' in an StringSearch expression needs to be a string"
   )
   CHECK_SILO_QUERY(
      json.contains("searchExpression"),
      "The field 'searchExpression' is required in an StringSearch expression"
   )
   CHECK_SILO_QUERY(
      json["searchExpression"].is_string(),
      "The field 'searchExpression' in an StringSearch expression needs to be a string"
   )
   const std::string& column = json["column"];
   const std::string& search_expression_string = json["searchExpression"].get<std::string>();
   auto search_expression = std::make_unique<re2::RE2>(search_expression_string);
   CHECK_SILO_QUERY(
      search_expression->ok(),
      "Invalid Regular Expression. The parsing of the regular expression failed with the error "
      "'{}'. See https://github.com/google/re2/wiki/Syntax for a Syntax specification.",
      search_expression->error()
   )
   filter = std::make_unique<StringSearch>(column, std::move(search_expression));
}

}  // namespace silo::query_engine::filter::expressions
