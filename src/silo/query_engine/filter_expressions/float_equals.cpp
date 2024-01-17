#include "silo/query_engine/filter_expressions/float_equals.h"

#include <cmath>
#include <memory>
#include <utility>

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

FloatEquals::FloatEquals(std::string column, double value)
    : column(std::move(column)),
      value(value) {}

std::string FloatEquals::toString(const silo::Database& /*database*/) const {
   return column + " = '" + std::to_string(value) + "'";
}

std::unique_ptr<silo::query_engine::operators::Operator> FloatEquals::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   silo::query_engine::filter_expressions::Expression::AmbiguityMode /*mode*/
) const {
   if (!database_partition.columns.float_columns.contains(column)) {
      return std::make_unique<operators::Empty>(database_partition.sequence_count);
   }

   const auto& float_column = database_partition.columns.float_columns.at(column);

   return std::make_unique<operators::Selection>(
      std::make_unique<operators::CompareToValueSelection<double>>(
         float_column.getValues(), operators::Comparator::EQUALS, value
      ),
      database_partition.sequence_count
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FloatEquals>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in an FloatEquals expression"
   )
   CHECK_SILO_QUERY(
      json["column"].is_string(), "The field 'column' in an FloatEquals expression must be a string"
   )
   CHECK_SILO_QUERY(
      json.contains("value"), "The field 'value' is required in an FloatEquals expression"
   )
   CHECK_SILO_QUERY(
      json["value"].is_number_float() || json["value"].is_null(),
      "The field 'value' in an FloatEquals expression must be a float"
   )
   const std::string& column = json["column"];
   const double& value = json["value"].is_null() ? std::nan("") : json["value"].get<double>();
   filter = std::make_unique<FloatEquals>(column, value);
}

}  // namespace silo::query_engine::filter_expressions
