#include "silo/query_engine/scalar_expressions/string_search.h"

#include <utility>

#include <fmt/format.h>

#include "silo/common/panic.h"
#include "silo/query_engine/filter/operators/bitmap_producer.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"

namespace silo::query_engine::scalar_expressions {

StringSearch::StringSearch(std::string column_name, std::unique_ptr<re2::RE2> search_expression)
    : column_name(std::move(column_name)),
      search_expression(std::move(search_expression)) {}

std::string StringSearch::toString() const {
   return fmt::format("column {} regex_matches \"{}\"", column_name, search_expression->pattern());
}

namespace {
template <typename GenericStringColumn>
std::unique_ptr<filter::operators::Operator> createMatchingBitmap(
   const GenericStringColumn& string_column,
   const RE2& search_expression,
   storage::column::RowLayout row_layout
) {
   auto producer = [&string_column, &search_expression, row_layout]() {
      roaring::Roaring result_bitmap;
      for (const auto row_id : row_layout) {
         const std::string full_string = string_column.getValueString(row_id);
         if (re2::RE2::PartialMatch(full_string, search_expression)) {
            result_bitmap.add(row_id.toGlobal());
         }
      }
      return CopyOnWriteBitmap(std::move(result_bitmap));
   };
   return std::make_unique<filter::operators::BitmapProducer>(
      std::move(producer), std::move(row_layout)
   );
}

}  // namespace

std::unique_ptr<ScalarExpression> StringSearch::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<StringSearch>(
      column_name, std::make_unique<re2::RE2>(search_expression->pattern())
   );
}

std::unique_ptr<filter::operators::Operator> StringSearch::compile(const storage::Table& table
) const {
   CHECK_SILO_QUERY(
      table.columns.string_columns.contains(column_name) ||
         table.columns.indexed_string_columns.contains(column_name),
      "The database does not contain the string column '{}'",
      column_name
   )

   if (table.columns.indexed_string_columns.contains(column_name)) {
      const auto& string_column = table.columns.indexed_string_columns.at(column_name);
      return createMatchingBitmap(string_column, *search_expression, table.row_layout);
   }
   SILO_ASSERT(table.columns.string_columns.contains(column_name));
   const auto& string_column = table.columns.string_columns.at(column_name);
   return createMatchingBitmap(string_column, *search_expression, table.row_layout);
}

}  // namespace silo::query_engine::scalar_expressions
