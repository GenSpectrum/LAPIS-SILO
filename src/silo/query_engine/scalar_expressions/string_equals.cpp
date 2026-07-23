#include "silo/query_engine/scalar_expressions/string_equals.h"
#include <fmt/format.h>

#include <optional>
#include <utility>

#include "silo/common/panic.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/scalar_expressions/is_null.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"
#include "silo/query_engine/scalar_expressions/string_in_set.h"

namespace silo::query_engine::scalar_expressions {

StringEquals::StringEquals(schema::ColumnIdentifier column, std::optional<std::string> value)
    : column(std::move(column)),
      value(std::move(value)) {}

std::string StringEquals::toString() const {
   if (value.has_value()) {
      return fmt::format("{} = '{}'", column.name, value.value());
   }
   return fmt::format("{} IS NULL", column.name);
}

std::vector<schema::ColumnIdentifier> StringEquals::freeIUs() const {
   return {column};
}

std::unique_ptr<ScalarExpression> StringEquals::rewrite(
   const storage::Table& table,
   AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      table.schema->getColumn(column.name).has_value(),
      "The database does not contain the column '{}'",
      column.name
   );
   CHECK_SILO_QUERY(
      table.columns.string_columns.contains(column.name) ||
         table.columns.indexed_string_columns.contains(column.name),
      "The column '{}' is not of type string",
      column.name
   );

   if (value == std::nullopt) {
      return std::make_unique<IsNull>(column);
   }

   // We do not change expressions for IndexedStringColumn
   if (table.columns.indexed_string_columns.contains(column.name)) {
      return std::make_unique<StringEquals>(column, value);
   }

   SILO_ASSERT(table.columns.string_columns.contains(column.name));

   return std::make_unique<StringInSet>(column, std::unordered_set<std::string>{value.value()});
}

std::unique_ptr<filter::operators::Operator> StringEquals::compile(const storage::Table& table
) const {
   // If it was a StringColumn it should have been rewritten
   SILO_ASSERT(table.columns.indexed_string_columns.contains(column.name));
   const auto& string_column = table.columns.indexed_string_columns.at(column.name);
   const auto bitmap = string_column.filter(value);

   if (bitmap == std::nullopt || bitmap.value()->isEmpty()) {
      return std::make_unique<filter::operators::Empty>(table.row_layout);
   }
   return std::make_unique<filter::operators::IndexScan>(
      CopyOnWriteBitmap{bitmap.value()}, table.row_layout
   );
}

}  // namespace silo::query_engine::scalar_expressions
