#include "silo/query_engine/filter/expressions/float_between.h"

#include <cmath>
#include <memory>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter::expressions {

// NOLINTBEGIN(bugprone-easily-swappable-parameters,readability-identifier-length)
FloatBetween::FloatBetween(
   std::string column_name,
   std::optional<double> from,
   std::optional<double> to
)
    // NOLINTEND(bugprone-easily-swappable-parameters,readability-identifier-length)
    : column_name(std::move(column_name)),
      from(from),
      to(to) {}

std::string FloatBetween::toString() const {
   const auto from_string = from.has_value() ? std::to_string(from.value()) : "unbounded";
   const auto to_string = to.has_value() ? std::to_string(to.value()) : "unbounded";

   return "[FloatBetween " + from_string + " - " + to_string + "]";
}

std::unique_ptr<silo::query_engine::filter::operators::Operator> FloatBetween::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   silo::query_engine::filter::expressions::Expression::AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      database_partition.columns.float_columns.contains(column_name),
      "The database does not contain the float column '" + column_name + "'"
   );
   const auto& float_column = database_partition.columns.float_columns.at(column_name);

   std::vector<std::unique_ptr<operators::Predicate>> predicates;
   if (from.has_value()) {
      predicates.emplace_back(std::make_unique<operators::CompareToValueSelection<double>>(
         float_column.getValues(), operators::Comparator::HIGHER_OR_EQUALS, from.value()
      ));
   }

   if (to.has_value()) {
      predicates.emplace_back(std::make_unique<operators::CompareToValueSelection<double>>(
         float_column.getValues(), operators::Comparator::LESS, to.value()
      ));
   }

   if (predicates.empty()) {
      predicates.emplace_back(std::make_unique<operators::CompareToValueSelection<double>>(
         float_column.getValues(),
         operators::Comparator::NOT_EQUALS,
         storage::column::FloatColumn::null()
      ));
   }

   return std::make_unique<operators::Selection>(
      std::move(predicates), database_partition.sequence_count
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FloatBetween>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in a FloatBetween expression"
   );
   CHECK_SILO_QUERY(
      json["column"].is_string(), "The field 'column' in a FloatBetween expression must be a string"
   );
   CHECK_SILO_QUERY(
      json.contains("from"), "The field 'from' is required in FloatBetween expression"
   );
   CHECK_SILO_QUERY(
      json["from"].is_null() || json["from"].is_number_float(),
      "The field 'from' in a FloatBetween expression must be a float or null"
   );
   CHECK_SILO_QUERY(json.contains("to"), "The field 'to' is required in a FloatBetween expression");
   CHECK_SILO_QUERY(
      json["to"].is_null() || json["to"].is_number_float(),
      "The field 'to' in a FloatBetween expression must be a float or null"
   );
   const std::string& column_name = json["column"];
   std::optional<double> value_from;
   if (json["from"].is_number_float()) {
      value_from = json["from"].get<double>();
   }
   std::optional<double> value_to;
   if (json["to"].is_number_float()) {
      value_to = json["to"].get<double>();
   }
   filter = std::make_unique<FloatBetween>(column_name, value_from, value_to);
}

}  // namespace silo::query_engine::filter::expressions
