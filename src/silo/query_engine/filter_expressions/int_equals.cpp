#include "silo/query_engine/filter_expressions/int_equals.h"

#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/selection.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/database_partition.h"

namespace silo {
class Database;
namespace query_engine::operators {
class Operator;
}  // namespace query_engine::operators
}  // namespace silo

namespace silo::query_engine::filter_expressions {

IntEquals::IntEquals(std::string column_name, uint32_t value)
    : column_name(std::move(column_name)),
      value(value) {}

std::string IntEquals::toString() const {
   return column_name + " = '" + std::to_string(value) + "'";
}

std::unique_ptr<silo::query_engine::operators::Operator> IntEquals::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      database_partition.columns.int_columns.contains(column_name),
      fmt::format("The database does not contain the column '{}'", column_name)
   );

   const auto& int_column = database_partition.columns.int_columns.at(column_name);

   return std::make_unique<operators::Selection>(
      std::make_unique<operators::CompareToValueSelection<int32_t>>(
         int_column.getValues(), operators::Comparator::EQUALS, value
      ),
      database_partition.sequence_count
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
   CHECK_SILO_QUERY(
      json["value"].is_number_integer() || json["value"].is_null(),
      "The field 'value' in an IntEquals expression must be an integer or null"
   );
   const std::string& column = json["column"];
   const int32_t& value = json["value"].is_null() ? INT32_MIN : json["value"].get<int32_t>();
   filter = std::make_unique<IntEquals>(column, value);
}

}  // namespace silo::query_engine::filter_expressions
