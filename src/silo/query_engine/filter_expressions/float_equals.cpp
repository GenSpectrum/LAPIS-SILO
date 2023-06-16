#include "silo/query_engine/filter_expressions/float_equals.h"

#include <nlohmann/json.hpp>

#include "silo/database.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/selection.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

FloatEquals::FloatEquals(std::string column, double value)
    : column(std::move(column)),
      value(value) {}

std::string FloatEquals::toString(const silo::Database& /*database*/) const {
   return column + " = '" + std::to_string(value) + "'";
}

std::unique_ptr<silo::query_engine::operators::Operator> FloatEquals::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   silo::query_engine::filter_expressions::Expression::AmbiguityMode mode
) const {
   if (!database_partition.meta_store.float_columns.contains(column)) {
      return std::make_unique<operators::Empty>(database_partition.sequenceCount);
   }

   const auto& float_column = database_partition.meta_store.float_columns.at(column);

   return std::make_unique<operators::Selection<double>>(
      float_column.getValues(),
      operators::Selection<double>::EQUALS,
      value,
      database_partition.sequenceCount
   );
}

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
      json["value"].is_number_float(),
      "The field 'value' in an FloatEquals expression must be a float"
   )
   const std::string& column = json["column"];
   const double& value = json["value"];
   filter = std::make_unique<FloatEquals>(column, value);
}

}  // namespace silo::query_engine::filter_expressions