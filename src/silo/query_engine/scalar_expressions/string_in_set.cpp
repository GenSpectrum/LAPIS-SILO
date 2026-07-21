#include "silo/query_engine/scalar_expressions/string_in_set.h"

#include <utility>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "silo/common/panic.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/string_in_set.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/scalar_expressions/or.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"
#include "silo/query_engine/scalar_expressions/string_equals.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/string_column.h"

namespace silo::query_engine::scalar_expressions {

using storage::column::StringColumn;

StringInSet::StringInSet(schema::ColumnIdentifier column, std::unordered_set<std::string> values)
    : column(std::move(column)),
      values(std::move(values)) {}

std::string StringInSet::toString() const {
   std::vector<std::string> sorted_values;
   std::ranges::copy(values, std::back_inserter(sorted_values));
   std::ranges::sort(sorted_values);
   return fmt::format("{} IN [{}]", column.name, fmt::join(sorted_values, ","));
}

std::vector<schema::ColumnIdentifier> StringInSet::freeIUs() const {
   return {column};
}

std::unique_ptr<ScalarExpression> StringInSet::rewrite(
   const storage::Table& table,
   AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      table.columns.string_columns.contains(column.name) ||
         table.columns.indexed_string_columns.contains(column.name),
      "The database does not contain the string column '{}'",
      column.name
   );

   // We do not change expressions for StringColumn
   if (table.columns.string_columns.contains(column.name)) {
      return std::make_unique<StringInSet>(column, values);
   }

   // We want to improve IndexedStringColumn by using our Indexes directly -> StringEquals
   std::vector<std::unique_ptr<ScalarExpression>> string_equal_expressions;
   string_equal_expressions.reserve(values.size());
   for (const auto& value : values) {
      string_equal_expressions.emplace_back(std::make_unique<StringEquals>(column, value));
   }
   return std::make_unique<Or>(std::move(string_equal_expressions));
}

std::unique_ptr<filter::operators::Operator> StringInSet::compile(const storage::Table& table
) const {
   SILO_ASSERT(table.columns.string_columns.contains(column.name));
   const auto& string_column = table.columns.string_columns.at(column.name);
   return std::make_unique<filter::operators::Selection>(
      std::make_unique<filter::operators::StringInSet<StringColumn>>(
         &string_column, filter::operators::StringInSet<StringColumn>::Comparator::IN, values
      ),
      table.row_layout
   );
}

}  // namespace silo::query_engine::scalar_expressions
