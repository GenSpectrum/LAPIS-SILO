#include "silo/query_engine/filter/expressions/date_between.h"

#include <cstdint>
#include <limits>
#include <map>
#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/common/date.h"
#include "silo/query_engine/filter/operators/range_selection.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/table_partition.h"

using silo::storage::column::DateColumnPartition;

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
   res += (date_from.has_value() ? silo::common::dateToString(date_from.value()) : "unbounded");
   res += " and ";
   res += (date_to.has_value() ? silo::common::dateToString(date_to.value()) : "unbounded");
   res += "]";
   return res;
}

std::unique_ptr<Expression> DateBetween::rewrite(
   const storage::Table& /*table*/,
   const storage::TablePartition& /*table_partition*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<DateBetween>(column_name, date_from, date_to);
}

std::unique_ptr<operators::Operator> DateBetween::compile(
   const storage::Table& table,
   const storage::TablePartition& table_partition
) const {
   CHECK_SILO_QUERY(
      table.schema.getColumn(column_name).has_value(),
      "The database does not contain the column '{}'",
      column_name
   );
   CHECK_SILO_QUERY(
      table_partition.columns.date_columns.contains(column_name),
      "The column '{}' is not of type date",
      column_name
   );

   const auto& date_column = table_partition.columns.date_columns.at(column_name);

   if (date_column.isSorted()) {
      return std::make_unique<operators::RangeSelection>(
         computeRangesOfSortedColumn(date_column, {table_partition.sequence_count}),
         table_partition.sequence_count
      );
   }
   operators::PredicateVector predicates;
   predicates.emplace_back(
      std::make_unique<operators::CompareToValueSelection<DateColumnPartition>>(
         date_column,
         operators::Comparator::HIGHER_OR_EQUALS,
         date_from.value_or(std::numeric_limits<silo::common::Date>::min())
      )
   );
   predicates.emplace_back(
      std::make_unique<operators::CompareToValueSelection<DateColumnPartition>>(
         date_column,
         operators::Comparator::LESS_OR_EQUALS,
         date_to.value_or(std::numeric_limits<silo::common::Date>::max())
      )
   );
   return std::make_unique<operators::Selection>(
      std::move(predicates), table_partition.sequence_count
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
      const auto* lower = std::lower_bound(
         begin, end, date_from.value_or(std::numeric_limits<silo::common::Date>::min())
      );
      const auto lower_index = static_cast<uint32_t>(lower - base);
      const auto* upper = date_to.has_value() ? std::upper_bound(begin, end, date_to.value()) : end;
      const auto upper_index = static_cast<uint32_t>(upper - base);
      ranges.emplace_back(lower_index, upper_index);
      offset += sorted_chunk_size;
   }
   return ranges;
}

// NOLINTNEXTLINE(readability-identifier-naming,readability-function-cognitive-complexity)
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
   if (json["from"].is_string()) {
      const auto from_string = json["from"].get<std::string>();
      auto from_result = common::stringToDate(from_string);
      CHECK_SILO_QUERY(
         from_result.has_value(), "Invalid date in 'from' field: {}", from_result.error()
      );
      date_from = from_result.value();
   }
   std::optional<silo::common::Date> date_to;
   if (json["to"].is_string()) {
      const auto to_string = json["to"].get<std::string>();
      auto to_result = common::stringToDate(to_string);
      CHECK_SILO_QUERY(to_result.has_value(), "Invalid date in 'to' field: {}", to_result.error());
      date_to = to_result.value();
   }
   filter = std::make_unique<DateBetween>(column_name, date_from, date_to);
}

}  // namespace silo::query_engine::filter::expressions
