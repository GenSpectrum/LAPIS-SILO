#include "silo/query_engine/expressions/float_between.h"

#include <cmath>
#include <memory>
#include <utility>
#include <vector>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"

using silo::storage::column::FloatColumn;

namespace silo::query_engine::expressions {

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

std::vector<schema::ColumnIdentifier> FloatBetween::freeIUs() const {
   return {{column_name, schema::ColumnType::BOOL}};
}

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

std::unique_ptr<filter::operators::Operator> FloatBetween::compile(const storage::Table& table
) const {
   CHECK_SILO_QUERY(
      table.columns.float_columns.contains(column_name),
      "The database does not contain the float column '{}'",
      column_name
   );
   const auto& float_column = table.columns.float_columns.at(column_name);

   filter::operators::PredicateVector predicates;
   if (from.has_value()) {
      predicates.emplace_back(
         std::make_unique<filter::operators::CompareToValueSelection<FloatColumn>>(
            float_column, filter::operators::Comparator::HIGHER_OR_EQUALS, from.value()
         )
      );
   }

   if (to.has_value()) {
      predicates.emplace_back(
         std::make_unique<filter::operators::CompareToValueSelection<FloatColumn>>(
            float_column, filter::operators::Comparator::LESS, to.value()
         )
      );
   }

   if (predicates.empty()) {
      return std::make_unique<filter::operators::Complement>(
         std::make_unique<filter::operators::IndexScan>(
            CopyOnWriteBitmap{&float_column.null_bitmap}, table.row_layout
         ),
         table.row_layout
      );
   }

   return std::make_unique<filter::operators::Selection>(std::move(predicates), table.row_layout);
}

}  // namespace silo::query_engine::expressions
