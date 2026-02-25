#include "silo/query_engine/filter/expressions/float_equals.h"

#include <cmath>
#include <memory>
#include <utility>

#include <fmt/format.h>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"

using silo::storage::column::FloatColumn;

namespace silo::query_engine::filter::expressions {

FloatEquals::FloatEquals(std::string column_name, std::optional<double> value)
    : column_name(std::move(column_name)),
      value(value) {}

std::string FloatEquals::toString() const {
   if (value.has_value()) {
      return fmt::format("{} = '{}'", column_name, value.value());
   }
   return fmt::format("{} IS NULL", column_name);
}

std::unique_ptr<Expression> FloatEquals::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<FloatEquals>(column_name, value);
}

std::unique_ptr<operators::Operator> FloatEquals::compile(const storage::Table& table) const {
   CHECK_SILO_QUERY(
      table.columns.float_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );

   const auto& float_column = table.columns.float_columns.at(column_name);

   if (value.has_value()) {
      return std::make_unique<operators::Selection>(
         std::make_unique<operators::CompareToValueSelection<FloatColumn>>(
            float_column, operators::Comparator::EQUALS, value.value()
         ),
         table.sequence_count
      );
   }
   return std::make_unique<operators::IndexScan>(
      CopyOnWriteBitmap{&float_column.null_bitmap}, table.sequence_count
   );
}

}  // namespace silo::query_engine::filter::expressions
