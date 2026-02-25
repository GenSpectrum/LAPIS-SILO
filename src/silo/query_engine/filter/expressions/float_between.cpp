#include "silo/query_engine/filter/expressions/float_between.h"

#include <cmath>
#include <memory>
#include <utility>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"

using silo::storage::column::FloatColumn;

namespace silo::query_engine::filter::expressions {

// NOLINTBEGIN(bugprone-easily-swappable-parameters,readability-identifier-length)
FloatBetween::FloatBetween(
   std::string column_name,
   std::optional<double> from,
   std::optional<double> to
)
    // NOLINTEND(bugprone-easily-swappable-parameters,readability-identifier-length)
    : column_name(std::move(column_name)),
      from(from),
      to(to) {}

std::string FloatBetween::toString() const {
   const auto from_string = from.has_value() ? std::to_string(from.value()) : "unbounded";
   const auto to_string = to.has_value() ? std::to_string(to.value()) : "unbounded";

   return "[FloatBetween " + from_string + " - " + to_string + "]";
}

std::unique_ptr<Expression> FloatBetween::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<FloatBetween>(column_name, from, to);
}

std::unique_ptr<operators::Operator> FloatBetween::compile(const storage::Table& table) const {
   CHECK_SILO_QUERY(
      table.columns.float_columns.contains(column_name),
      "The database does not contain the float column '{}'",
      column_name
   );
   const auto& float_column = table.columns.float_columns.at(column_name);

   operators::PredicateVector predicates;
   if (from.has_value()) {
      predicates.emplace_back(std::make_unique<operators::CompareToValueSelection<FloatColumn>>(
         float_column, operators::Comparator::HIGHER_OR_EQUALS, from.value()
      ));
   }

   if (to.has_value()) {
      predicates.emplace_back(std::make_unique<operators::CompareToValueSelection<FloatColumn>>(
         float_column, operators::Comparator::LESS, to.value()
      ));
   }

   if (predicates.empty()) {
      return std::make_unique<operators::Complement>(
         std::make_unique<operators::IndexScan>(
            CopyOnWriteBitmap{&float_column.null_bitmap}, table.sequence_count
         ),
         table.sequence_count
      );
   }

   return std::make_unique<operators::Selection>(std::move(predicates), table.sequence_count);
}

}  // namespace silo::query_engine::filter::expressions
