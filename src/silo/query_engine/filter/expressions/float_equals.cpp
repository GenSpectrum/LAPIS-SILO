#include "silo/query_engine/filter/expressions/float_equals.h"

#include <cmath>
#include <memory>
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

FloatEquals::FloatEquals(std::string column_name, double value)
    : column_name(std::move(column_name)),
      value(value) {}

std::string FloatEquals::toString() const {
   return fmt::format("{} = '{}'", column_name, std::to_string(value));
}

std::unique_ptr<silo::query_engine::filter::operators::Operator> FloatEquals::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition,
   silo::query_engine::filter::expressions::Expression::AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      table_partition.columns.float_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );

   const auto& float_column = table_partition.columns.float_columns.at(column_name);

   return std::make_unique<operators::Selection>(
      std::make_unique<operators::CompareToValueSelection<double>>(
         float_column.getValues(), operators::Comparator::EQUALS, value
      ),
      table_partition.sequence_count
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FloatEquals>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in an FloatEquals expression"
   );
   CHECK_SILO_QUERY(
      json["column"].is_string(), "The field 'column' in an FloatEquals expression must be a string"
   );
   CHECK_SILO_QUERY(
      json.contains("value"), "The field 'value' is required in an FloatEquals expression"
   );
   CHECK_SILO_QUERY(
      json["value"].is_number_float() || json["value"].is_null(),
      "The field 'value' in an FloatEquals expression must be a float or null"
   );
   const std::string& column_name = json["column"];
   const double& value = json["value"].is_null() ? storage::column::FloatColumnPartition::null()
                                                 : json["value"].get<double>();
   filter = std::make_unique<FloatEquals>(column_name, value);
}

}  // namespace silo::query_engine::filter::expressions
