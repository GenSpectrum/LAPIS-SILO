#include "silo/query_engine/scalar_expressions/bool_equals.h"

#include <utility>

#include <fmt/format.h>

#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"

namespace silo::query_engine::scalar_expressions {

BoolEquals::BoolEquals(schema::ColumnIdentifier column, std::optional<bool> value)
    : column(std::move(column)),
      value(value) {}

std::string BoolEquals::toString() const {
   if (value.has_value()) {
      return fmt::format("{} = {}", column.name, value.value() ? "true" : "false");
   }
   return fmt::format("{} IS NULL", column.name);
}

std::vector<schema::ColumnIdentifier> BoolEquals::freeIUs() const {
   return {column};
}

std::unique_ptr<ScalarExpression> BoolEquals::rewrite(
   const storage::Table& /*table*/,
   ScalarExpression::AmbiguityMode /*mode*/
) const {
   return std::make_unique<BoolEquals>(column, value);
}

std::unique_ptr<filter::operators::Operator> BoolEquals::compile(const storage::Table& table
) const {
   CHECK_SILO_QUERY(
      table.schema->getColumn(column.name).has_value(),
      "The database does not contain the column '{}'",
      column.name
   );
   CHECK_SILO_QUERY(
      table.columns.bool_columns.contains(column.name),
      "The column '{}' is not of type bool",
      column.name
   );

   const auto& bool_column = table.columns.bool_columns.at(column.name);

   if (value == std::nullopt) {
      return std::make_unique<filter::operators::IndexScan>(
         CopyOnWriteBitmap{&bool_column.null_bitmap}, table.row_layout
      );
   }
   if (value.value()) {
      return std::make_unique<filter::operators::IndexScan>(
         CopyOnWriteBitmap{&bool_column.true_bitmap}, table.row_layout
      );
   }
   return std::make_unique<filter::operators::IndexScan>(
      CopyOnWriteBitmap{&bool_column.false_bitmap}, table.row_layout
   );

   SILO_UNREACHABLE();
}

}  // namespace silo::query_engine::scalar_expressions
