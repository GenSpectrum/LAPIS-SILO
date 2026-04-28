#include "silo/query_engine/filter/expressions/date_between.h"

#include <cstdint>
#include <limits>
#include <map>
#include <utility>

#include <fmt/format.h>

#include "silo/common/date32.h"
#include "silo/query_engine/filter/operators/range_selection.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/storage/column/date32_column.h"

using silo::storage::column::Date32Column;

namespace silo::query_engine::filter::expressions {

DateBetween::DateBetween(
   std::string column_name,
   // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
   std::optional<silo::common::Date32> date_from,
   std::optional<silo::common::Date32> date_to
)
    : column_name(std::move(column_name)),
      date_from(date_from),
      date_to(date_to) {}

std::string DateBetween::toString() const {
   std::string res = "[Date-between ";
   res += (date_from.has_value() ? silo::common::date32ToString(date_from.value()) : "unbounded");
   res += " and ";
   res += (date_to.has_value() ? silo::common::date32ToString(date_to.value()) : "unbounded");
   res += "]";
   return res;
}

std::unique_ptr<Expression> DateBetween::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<DateBetween>(column_name, date_from, date_to);
}

std::unique_ptr<operators::Operator> DateBetween::compile(const storage::Table& table) const {
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

   if (date_column.isSorted()) {
      return std::make_unique<operators::RangeSelection>(
         computeRangesOfSortedColumn(date_column, {table.sequence_count}), table.sequence_count
      );
   }
   operators::PredicateVector predicates;
   predicates.emplace_back(std::make_unique<operators::CompareToValueSelection<Date32Column>>(
      date_column,
      operators::Comparator::HIGHER_OR_EQUALS,
      date_from.value_or(std::numeric_limits<silo::common::Date32>::min())
   ));
   predicates.emplace_back(std::make_unique<operators::CompareToValueSelection<Date32Column>>(
      date_column,
      operators::Comparator::LESS_OR_EQUALS,
      date_to.value_or(std::numeric_limits<silo::common::Date32>::max())
   ));
   return std::make_unique<operators::Selection>(std::move(predicates), table.sequence_count);
}

std::vector<silo::query_engine::filter::operators::RangeSelection::Range> DateBetween::
   computeRangesOfSortedColumn(
      const silo::storage::column::Date32Column& date_column,
      const std::vector<size_t>& chunk_sizes
   ) const {
   std::vector<operators::RangeSelection::Range> ranges;

   size_t offset = 0;

   const auto* base = date_column.getValues().data();
   for (const auto& sorted_chunk_size : chunk_sizes) {
      const auto* begin = &date_column.getValues()[offset];
      const auto* end = &date_column.getValues()[offset + sorted_chunk_size];
      const auto* lower = std::lower_bound(
         begin, end, date_from.value_or(std::numeric_limits<silo::common::Date32>::min())
      );
      const auto lower_index = static_cast<uint32_t>(lower - base);
      const auto* upper = date_to.has_value() ? std::upper_bound(begin, end, date_to.value()) : end;
      const auto upper_index = static_cast<uint32_t>(upper - base);
      ranges.emplace_back(lower_index, upper_index);
      offset += sorted_chunk_size;
   }
   return ranges;
}

}  // namespace silo::query_engine::filter::expressions
