#include "silo/query_engine/scalar_expressions/float_equals.h"

#include <cmath>
#include <memory>
#include <utility>

#include <fmt/format.h>

#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"

using silo::storage::column::FloatColumn;

namespace silo::query_engine::scalar_expressions {

FloatEquals::FloatEquals(std::string column_name, std::optional<double> value)
    : column_name(std::move(column_name)),
      value(value) {}

std::string FloatEquals::toString() const {
   if (value.has_value()) {
      return fmt::format("{} = '{}'", column_name, value.value());
   }
   return fmt::format("{} IS NULL", column_name);
}

std::unique_ptr<ScalarExpression> FloatEquals::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<FloatEquals>(column_name, value);
}

std::unique_ptr<filter::operators::Operator> FloatEquals::compile(const storage::Table& table
) const {
   CHECK_SILO_QUERY(
      table.schema->getColumn(column_name).has_value(),
      "The database does not contain the column '{}'",
      column_name
   );
   CHECK_SILO_QUERY(
      table.columns.float_columns.contains(column_name),
      "The column '{}' is not of type float",
      column_name
   );

   const auto& float_column = table.columns.float_columns.at(column_name);

   if (value.has_value()) {
      return std::make_unique<filter::operators::Selection>(
         std::make_unique<filter::operators::CompareToValueSelection<FloatColumn>>(
            float_column, filter::operators::Comparator::EQUALS, value.value()
         ),
         table.row_layout
      );
   }
   return std::make_unique<filter::operators::IndexScan>(
      CopyOnWriteBitmap{&float_column.null_bitmap}, table.row_layout
   );
}

}  // namespace silo::query_engine::scalar_expressions
