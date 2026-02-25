#include "silo/query_engine/filter/expressions/string_in_set.h"

#include <utility>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "silo/common/panic.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/expressions/or.h"
#include "silo/query_engine/filter/expressions/string_equals.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/string_in_set.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/string_column.h"

namespace silo::query_engine::filter::expressions {

using storage::column::StringColumn;

StringInSet::StringInSet(std::string column_name, std::unordered_set<std::string> values)
    : column_name(std::move(column_name)),
      values(std::move(values)) {}

std::string StringInSet::toString() const {
   std::vector<std::string> sorted_values;
   std::ranges::copy(values, std::back_inserter(sorted_values));
   std::ranges::sort(sorted_values);
   return fmt::format("{} IN [{}]", column_name, fmt::join(sorted_values, ","));
}

std::unique_ptr<Expression> StringInSet::rewrite(
   const storage::Table& table,
   AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      table.columns.string_columns.contains(column_name) ||
         table.columns.indexed_string_columns.contains(column_name),
      "The database does not contain the string column '{}'",
      column_name
   );

   // We do not change expressions for StringColumn
   if (table.columns.string_columns.contains(column_name)) {
      return std::make_unique<StringInSet>(column_name, values);
   }

   // We want to improve IndexedStringColumn by using our Indexes directly -> StringEquals
   std::vector<std::unique_ptr<Expression>> string_equal_expressions;
   string_equal_expressions.reserve(values.size());
   for (const auto& value : values) {
      string_equal_expressions.emplace_back(std::make_unique<StringEquals>(column_name, value));
   }
   return std::make_unique<Or>(std::move(string_equal_expressions));
}

std::unique_ptr<operators::Operator> StringInSet::compile(const storage::Table& table) const {
   SILO_ASSERT(table.columns.string_columns.contains(column_name));
   const auto& string_column = table.columns.string_columns.at(column_name);
   return std::make_unique<operators::Selection>(
      std::make_unique<operators::StringInSet<StringColumn>>(
         &string_column, operators::StringInSet<StringColumn>::Comparator::IN, values
      ),
      table.sequence_count
   );
}

}  // namespace silo::query_engine::filter::expressions
