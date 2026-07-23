#include "silo/query_engine/scalar_expressions/int_equals.h"

#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"

using silo::storage::column::IntColumn;

namespace silo::query_engine::scalar_expressions {

IntEquals::IntEquals(schema::ColumnIdentifier column, std::optional<int32_t> value)
    : column(std::move(column)),
      value(value) {}

std::string IntEquals::toString() const {
   if (value.has_value()) {
      return fmt::format("{} = '{}'", column.name, value.value());
   }
   return fmt::format("{} IS NULL", column.name);
}

std::vector<schema::ColumnIdentifier> IntEquals::freeIUs() const {
   return {column};
}

std::unique_ptr<ScalarExpression> IntEquals::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<IntEquals>(column, value);
}

std::unique_ptr<filter::operators::Operator> IntEquals::compile(const storage::Table& table) const {
   CHECK_SILO_QUERY(
      table.schema->getColumn(column.name).has_value(),
      "The database does not contain the column '{}'",
      column.name
   );
   CHECK_SILO_QUERY(
      table.columns.int_columns.contains(column.name),
      "The column '{}' is not of type int",
      column.name
   );

   const auto& int_column = table.columns.int_columns.at(column.name);

   if (value.has_value()) {
      return std::make_unique<filter::operators::Selection>(
         std::make_unique<filter::operators::CompareToValueSelection<IntColumn>>(
            int_column, filter::operators::Comparator::EQUALS, value.value()
         ),
         table.row_layout
      );
   }
   return std::make_unique<filter::operators::IndexScan>(
      CopyOnWriteBitmap{&int_column.null_bitmap}, table.row_layout
   );
}

}  // namespace silo::query_engine::scalar_expressions
