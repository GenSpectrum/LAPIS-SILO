#include "silo/query_engine/filter/expressions/date_equals.h"

#include <utility>

#include <fmt/format.h>

#include "silo/common/date32.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/storage/column/date32_column.h"

using silo::storage::column::Date32Column;

namespace silo::query_engine::filter::expressions {

DateEquals::DateEquals(std::string column_name, std::optional<silo::common::Date32> value)
    : column_name(std::move(column_name)),
      value(value) {}

std::string DateEquals::toString() const {
   if (value.has_value()) {
      const auto date_string = silo::common::date32ToString(value.value());
      return fmt::format("{} = '{}'", column_name, date_string);
   }
   return fmt::format("{} IS NULL", column_name);
}

std::unique_ptr<Expression> DateEquals::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<DateEquals>(column_name, value);
}

std::unique_ptr<operators::Operator> DateEquals::compile(const storage::Table& table) const {
   CHECK_SILO_QUERY(
      table.schema->getColumn(column_name).has_value(),
      "The database does not contain the column '{}'",
      column_name
   );
   CHECK_SILO_QUERY(
      table.columns.date32_columns.contains(column_name),
      "The column '{}' is not of type date",
      column_name
   );

   const auto& date_column = table.columns.date32_columns.at(column_name);

   if (value.has_value()) {
      return std::make_unique<operators::Selection>(
         std::make_unique<operators::CompareToValueSelection<Date32Column>>(
            date_column, operators::Comparator::EQUALS, value.value()
         ),
         table.sequence_count
      );
   }
   return std::make_unique<operators::IndexScan>(
      CopyOnWriteBitmap{&date_column.null_bitmap}, table.sequence_count
   );
}

}  // namespace silo::query_engine::filter::expressions
