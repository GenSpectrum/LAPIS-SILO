#include "silo/query_engine/expressions/int_equals.h"

#include <utility>
#include <vector>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"

using silo::storage::column::IntColumn;

namespace silo::query_engine::expressions {

IntEquals::IntEquals(std::string column_name, std::optional<int32_t> value)
    : column_name(std::move(column_name)),
      value(value) {}

std::vector<schema::ColumnIdentifier> IntEquals::freeIUs() const {
   return {{.name = column_name, .type = schema::ColumnType::BOOL}};
}

std::string IntEquals::toString() const {
   if (value.has_value()) {
      return fmt::format("{} = '{}'", column_name, value.value());
   }
   return fmt::format("{} IS NULL", column_name);
}

std::unique_ptr<Expression> IntEquals::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<IntEquals>(column_name, value);
}

std::unique_ptr<filter::operators::Operator> IntEquals::compile(const storage::Table& table) const {
   CHECK_SILO_QUERY(
      table.schema->getColumn(column_name).has_value(),
      "The database does not contain the column '{}'",
      column_name
   );
   CHECK_SILO_QUERY(
      table.columns.int_columns.contains(column_name),
      "The column '{}' is not of type int",
      column_name
   );

   const auto& int_column = table.columns.int_columns.at(column_name);

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

}  // namespace silo::query_engine::expressions
