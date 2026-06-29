#include "silo/query_engine/expressions/int_between.h"

#include <memory>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"

using silo::storage::column::IntColumn;

namespace silo::query_engine::expressions {

// NOLINTBEGIN(bugprone-easily-swappable-parameters,readability-identifier-length)
IntBetween::IntBetween(
   std::string column_name,
   std::optional<int32_t> from,
   std::optional<int32_t> to
)
    // NOLINTEND(bugprone-easily-swappable-parameters,readability-identifier-length)
    : column_name(std::move(column_name)),
      from(from),
      to(to) {}

std::vector<schema::ColumnIdentifier> IntBetween::freeIUs() const {
   return {{column_name, schema::ColumnType::BOOL}};
}

std::string IntBetween::toString() const {
   const auto from_string = from.has_value() ? std::to_string(from.value()) : "unbounded";
   const auto to_string = to.has_value() ? std::to_string(to.value()) : "unbounded";

   return "[IntBetween " + from_string + " - " + to_string + "]";
}

std::unique_ptr<Expression> IntBetween::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<IntBetween>(column_name, from, to);
}

std::unique_ptr<filter::operators::Operator> IntBetween::compile(const storage::Table& table
) const {
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

   filter::operators::PredicateVector predicates;
   if (from.has_value()) {
      predicates.emplace_back(
         std::make_unique<filter::operators::CompareToValueSelection<IntColumn>>(
            int_column, filter::operators::Comparator::HIGHER_OR_EQUALS, from.value()
         )
      );
   }
   if (to.has_value()) {
      predicates.emplace_back(
         std::make_unique<filter::operators::CompareToValueSelection<IntColumn>>(
            int_column, filter::operators::Comparator::LESS_OR_EQUALS, to.value()
         )
      );
   }

   if (predicates.empty()) {
      return std::make_unique<filter::operators::Complement>(
         std::make_unique<filter::operators::IndexScan>(
            CopyOnWriteBitmap{&int_column.null_bitmap}, table.row_layout
         ),
         table.row_layout
      );
   }

   auto result =
      std::make_unique<filter::operators::Selection>(std::move(predicates), table.row_layout);

   SPDLOG_TRACE("Compiled IntBetween filter expression to {}", result->toString());

   return std::move(result);
}

}  // namespace silo::query_engine::expressions
