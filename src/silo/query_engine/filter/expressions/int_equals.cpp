#include "silo/query_engine/filter/expressions/int_equals.h"

#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

IntEquals::IntEquals(std::string column_name, uint32_t value)
    : column_name(std::move(column_name)),
      value(value) {}

std::string IntEquals::toString() const {
   return column_name + " = '" + std::to_string(value) + "'";
}

std::unique_ptr<silo::query_engine::filter::operators::Operator> IntEquals::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      table_partition.columns.int_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );

   const auto& int_column = table_partition.columns.int_columns.at(column_name);

   return std::make_unique<operators::Selection>(
      std::make_unique<operators::CompareToValueSelection<int32_t>>(
         int_column.getValues(), operators::Comparator::EQUALS, value
      ),
      table_partition.sequence_count
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<IntEquals>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in an IntEquals expression"
   );
   CHECK_SILO_QUERY(
      json["column"].is_string(), "The field 'column' in an IntEquals expression must be a string"
   );
   CHECK_SILO_QUERY(
      json.contains("value"), "The field 'value' is required in an IntEquals expression"
   );
   bool value_in_allowed_range =
      json["value"].is_number_integer() &&
      json["value"].get<int32_t>() != storage::column::IntColumnPartition::null();
   CHECK_SILO_QUERY(
      value_in_allowed_range || json["value"].is_null(),
      "The field 'value' in an IntEquals expression must be an integer in [-2147483647; "
      "2147483647] or null"
   );
   const std::string& column = json["column"];
   const int32_t& value = json["value"].is_null() ? storage::column::IntColumnPartition::null()
                                                  : json["value"].get<int32_t>();
   filter = std::make_unique<IntEquals>(column, value);
}

}  // namespace silo::query_engine::filter::expressions
