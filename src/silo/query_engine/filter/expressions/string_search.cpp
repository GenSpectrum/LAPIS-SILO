#include "silo/query_engine/filter/expressions/string_search.h"

#include <utility>

#include <fmt/format.h>

#include "silo/common/panic.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/bitmap_producer.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"

namespace silo::query_engine::filter::expressions {

StringSearch::StringSearch(std::string column_name, std::unique_ptr<re2::RE2> search_expression)
    : column_name(std::move(column_name)),
      search_expression(std::move(search_expression)) {}

std::string StringSearch::toString() const {
   return fmt::format("column {} regex_matches \"{}\"", column_name, search_expression->pattern());
}

namespace {
template <typename GenericStringColumn>
std::unique_ptr<operators::Operator> createMatchingBitmap(
   const GenericStringColumn& string_column,
   const RE2& search_expression,
   size_t row_count
) {
   return std::make_unique<operators::BitmapProducer>(
      [&, row_count]() {
         roaring::Roaring result_bitmap;
         for (size_t row_idx = 0; row_idx < row_count; ++row_idx) {
            const std::string full_string = string_column.getValueString(row_idx);
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

std::unique_ptr<Expression> StringSearch::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<StringSearch>(
      column_name, std::make_unique<re2::RE2>(search_expression->pattern())
   );
}

std::unique_ptr<operators::Operator> StringSearch::compile(const storage::Table& table) const {
   CHECK_SILO_QUERY(
      table.columns.string_columns.contains(column_name) ||
         table.columns.indexed_string_columns.contains(column_name),
      "The database does not contain the string column '{}'",
      column_name
   )

   if (table.columns.indexed_string_columns.contains(column_name)) {
      const auto& string_column = table.columns.indexed_string_columns.at(column_name);
      return createMatchingBitmap(string_column, *search_expression, table.sequence_count);
   }
   SILO_ASSERT(table.columns.string_columns.contains(column_name));
   const auto& string_column = table.columns.string_columns.at(column_name);
   return createMatchingBitmap(string_column, *search_expression, table.sequence_count);
}

}  // namespace silo::query_engine::filter::expressions
