#include "silo/query_engine/filter/expressions/bool_equals.h"

#include <utility>

#include <fmt/format.h>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"

namespace silo::query_engine::filter::expressions {

BoolEquals::BoolEquals(std::string column_name, std::optional<bool> value)
    : column_name(std::move(column_name)),
      value(value) {}

std::string BoolEquals::toString() const {
   if (value.has_value()) {
      return fmt::format("{} = {}", column_name, value.value() ? "true" : "false");
   }
   return fmt::format("{} IS NULL", column_name);
}

std::unique_ptr<Expression> BoolEquals::rewrite(
   const storage::Table& /*table*/,
   Expression::AmbiguityMode /*mode*/
) const {
   return std::make_unique<BoolEquals>(column_name, value);
}

std::unique_ptr<operators::Operator> BoolEquals::compile(const storage::Table& table) const {
   CHECK_SILO_QUERY(
      table.columns.bool_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );

   const auto& bool_column = table.columns.bool_columns.at(column_name);

   if (value == std::nullopt) {
      return std::make_unique<operators::IndexScan>(
         CopyOnWriteBitmap{&bool_column.null_bitmap}, table.sequence_count
      );
   }
   if (value.value()) {
      return std::make_unique<operators::IndexScan>(
         CopyOnWriteBitmap{&bool_column.true_bitmap}, table.sequence_count
      );
   }
   return std::make_unique<operators::IndexScan>(
      CopyOnWriteBitmap{&bool_column.false_bitmap}, table.sequence_count
   );

   SILO_UNREACHABLE();
}

}  // namespace silo::query_engine::filter::expressions
