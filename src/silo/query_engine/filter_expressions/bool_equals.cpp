#include "silo/query_engine/filter_expressions/bool_equals.h"
#include <fmt/core.h>

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

using silo::common::OptionalBool;

BoolEquals::BoolEquals(std::string column, OptionalBool value)
    : column(std::move(column)),
      value(value) {}

std::string BoolEquals::toString() const {
   return fmt::format("{} = '{}'", column, value.asStr());
}

std::unique_ptr<silo::query_engine::operators::Operator> BoolEquals::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      database_partition.columns.bool_columns.contains(column),
      fmt::format("the database does not contain the column {}", column)
   );

   const auto& bool_column = database_partition.columns.bool_columns.at(column);

   return std::make_unique<operators::Selection>(
      std::make_unique<operators::CompareToValueSelection<OptionalBool>>(
         bool_column.getValues(), operators::Comparator::EQUALS, value
      ),
      database_partition.sequence_count
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<BoolEquals>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in an BoolEquals expression"
   )
   CHECK_SILO_QUERY(
      json["column"].is_string(), "The field 'column' in an BoolEquals expression must be a string"
   )
   CHECK_SILO_QUERY(
      json.contains("value"), "The field 'value' is required in an BoolEquals expression"
   )
   CHECK_SILO_QUERY(
      json["value"].is_boolean() || json["value"].is_null(),
      "The field 'value' in an BoolEquals expression must be a boolean or null"
   )
   const std::string& column = json["column"];
   const OptionalBool value =
      json["value"].is_null() ? OptionalBool() : OptionalBool(json["value"].get<bool>());
   filter = std::make_unique<BoolEquals>(column, value);
}

}  // namespace silo::query_engine::filter_expressions
