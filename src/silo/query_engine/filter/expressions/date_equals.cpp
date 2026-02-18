#include "silo/query_engine/filter/expressions/date_equals.h"

#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/common/date.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/table_partition.h"

using silo::storage::column::DateColumnPartition;

namespace silo::query_engine::filter::expressions {

DateEquals::DateEquals(std::string column_name, std::optional<silo::common::Date> value)
    : column_name(std::move(column_name)),
      value(value) {}

std::string DateEquals::toString() const {
   if (value.has_value()) {
      const auto date_string = silo::common::dateToString(value.value());
      return fmt::format("{} = '{}'", column_name, date_string.value_or(""));
   }
   return fmt::format("{} IS NULL", column_name);
}

std::unique_ptr<Expression> DateEquals::rewrite(
   const storage::Table& /*table*/,
   const storage::TablePartition& /*table_partition*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<DateEquals>(column_name, value);
}

std::unique_ptr<operators::Operator> DateEquals::compile(
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

   if (value.has_value()) {
      return std::make_unique<operators::Selection>(
         std::make_unique<operators::CompareToValueSelection<DateColumnPartition>>(
            date_column, operators::Comparator::EQUALS, value.value()
         ),
         table_partition.sequence_count
      );
   }
   return std::make_unique<operators::IndexScan>(
      CopyOnWriteBitmap{&date_column.null_bitmap}, table_partition.sequence_count
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<DateEquals>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in a DateEquals expression"
   );
   CHECK_SILO_QUERY(
      json["column"].is_string(), "The field 'column' in a DateEquals expression must be a string"
   );
   CHECK_SILO_QUERY(
      json.contains("value"), "The field 'value' is required in a DateEquals expression"
   );
   CHECK_SILO_QUERY(
      json["value"].is_null() || (json["value"].is_string() && !json["value"].empty()),
      "The field 'value' in a DateEquals expression must be a non-empty date string or null"
   );
   const std::string& column_name = json["column"];
   std::optional<silo::common::Date> value;
   if (json["value"].is_string()) {
      value = common::stringToDate(json["value"].get<std::string>());
   }
   filter = std::make_unique<DateEquals>(column_name, value);
}

}  // namespace silo::query_engine::filter::expressions
