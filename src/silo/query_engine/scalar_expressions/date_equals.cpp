#include "silo/query_engine/scalar_expressions/date_equals.h"

#include <utility>

#include <fmt/format.h>

#include "silo/common/date32.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"
#include "silo/storage/column/date32_column.h"

using silo::storage::column::Date32Column;

namespace silo::query_engine::scalar_expressions {

DateEquals::DateEquals(schema::ColumnIdentifier column, std::optional<silo::common::Date32> value)
    : column(std::move(column)),
      value(value) {}

std::string DateEquals::toString() const {
   if (value.has_value()) {
      const auto date_string = silo::common::date32ToString(value.value());
      return fmt::format("{} = '{}'", column.name, date_string);
   }
   return fmt::format("{} IS NULL", column.name);
}

std::vector<schema::ColumnIdentifier> DateEquals::freeIUs() const {
   return {column};
}

std::unique_ptr<ScalarExpression> DateEquals::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<DateEquals>(column, value);
}

std::unique_ptr<filter::operators::Operator> DateEquals::compile(const storage::Table& table
) const {
   CHECK_SILO_QUERY(
      table.schema->getColumn(column.name).has_value(),
      "The database does not contain the column '{}'",
      column.name
   );
   CHECK_SILO_QUERY(
      table.columns.date32_columns.contains(column.name),
      "The column '{}' is not of type date",
      column.name
   );

   const auto& date_column = table.columns.date32_columns.at(column.name);

   if (value.has_value()) {
      return std::make_unique<filter::operators::Selection>(
         std::make_unique<filter::operators::CompareToValueSelection<Date32Column>>(
            date_column, filter::operators::Comparator::EQUALS, value.value()
         ),
         table.row_layout
      );
   }
   return std::make_unique<filter::operators::IndexScan>(
      CopyOnWriteBitmap{&date_column.null_bitmap}, table.row_layout
   );
}

}  // namespace silo::query_engine::scalar_expressions
