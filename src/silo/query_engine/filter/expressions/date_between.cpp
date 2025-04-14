#include "silo/query_engine/filter/expressions/date_between.h"

#include <cstdint>
#include <map>
#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/common/date.h"
#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/operators/range_selection.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

DateBetween::DateBetween(
   std::string column_name,
   // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
   std::optional<silo::common::Date> date_from,
   std::optional<silo::common::Date> date_to
)
    : column_name(std::move(column_name)),
      date_from(date_from),
      date_to(date_to) {}

std::string DateBetween::toString() const {
   std::string res = "[Date-between ";
   res +=
      (date_from.has_value() ? silo::common::dateToString(date_from.value()).value_or("")
                             : "unbounded");
   res += " and ";
   res +=
      (date_to.has_value() ? silo::common::dateToString(date_to.value()).value_or("") : "unbounded"
      );
   res += "]";
   return res;
}

std::unique_ptr<operators::Operator> DateBetween::compile(
   const Database& /*database*/,
   const storage::TablePartition& database_partition,
   AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      database_partition.columns.date_columns.contains(column_name),
      fmt::format("The database does not contain the column '{}'", column_name)
   );

   const auto& date_column = database_partition.columns.date_columns.at(column_name);

   if (!date_column.isSorted()) {
      std::vector<std::unique_ptr<operators::Predicate>> predicates;
      predicates.emplace_back(
         std::make_unique<operators::CompareToValueSelection<silo::common::Date>>(
            date_column.getValues(),
            operators::Comparator::HIGHER_OR_EQUALS,
            date_from.value_or(silo::common::Date{1})
         )
      );
      predicates.emplace_back(
         std::make_unique<operators::CompareToValueSelection<silo::common::Date>>(
            date_column.getValues(),
            operators::Comparator::LESS_OR_EQUALS,
            date_to.value_or(silo::common::Date{UINT32_MAX})
         )
      );
      return std::make_unique<operators::Selection>(
         std::move(predicates), database_partition.sequence_count
      );
   }
   return std::make_unique<operators::RangeSelection>(
      computeRangesOfSortedColumn(date_column, {database_partition.sequence_count}),
      database_partition.sequence_count
   );
}

std::vector<silo::query_engine::filter::operators::RangeSelection::Range> DateBetween::
   computeRangesOfSortedColumn(
      const silo::storage::column::DateColumnPartition& date_column,
      const std::vector<size_t>& chunk_sizes
   ) const {
   std::vector<operators::RangeSelection::Range> ranges;

   size_t offset = 0;

   const auto* base = date_column.getValues().data();
   for (const auto& sorted_chunk_size : chunk_sizes) {
      const auto* begin = &date_column.getValues()[offset];
      const auto* end = &date_column.getValues()[offset + sorted_chunk_size];
      // If lower bound is empty we use 1 as the lower-bound, as 0 represents NULL values
      const auto* lower = std::lower_bound(begin, end, date_from.value_or(1));
      const uint32_t lower_index = lower - base;
      const auto* upper = date_to.has_value() ? std::upper_bound(begin, end, date_to.value()) : end;
      const uint32_t upper_index = upper - base;
      ranges.emplace_back(lower_index, upper_index);
      offset += sorted_chunk_size;
   }
   return ranges;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<DateBetween>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in a DateBetween expression"
   );
   CHECK_SILO_QUERY(
      json["column"].is_string(),
      "The field 'column' in a DateBetween expression needs to be a string"
   );
   CHECK_SILO_QUERY(
      json.contains("from"), "The field 'from' is required in DateBetween expression"
   );
   CHECK_SILO_QUERY(
      json["from"].is_null() || (json["from"].is_string() && !json["from"].empty()),
      "The field 'from' in a DateBetween expression needs to be a string or null"
   );
   CHECK_SILO_QUERY(json.contains("to"), "The field 'to' is required in a DateBetween expression");
   CHECK_SILO_QUERY(
      json["to"].is_null() || (json["to"].is_string() && !json["to"].empty()),
      "The field 'to' in a DateBetween expression needs to be a non-empty string or null"
   );
   const std::string& column_name = json["column"];
   std::optional<silo::common::Date> date_from;
   if (json["from"].type() == nlohmann::detail::value_t::string) {
      date_from = common::stringToDate(json["from"].get<std::string>());
   }
   std::optional<silo::common::Date> date_to;
   if (json["to"].type() == nlohmann::detail::value_t::string) {
      date_to = common::stringToDate(json["to"].get<std::string>());
   }
   filter = std::make_unique<DateBetween>(column_name, date_from, date_to);
}

}  // namespace silo::query_engine::filter::expressions
