#include "silo/query_engine/filter_expressions/int_between.h"

#include <nlohmann/json.hpp>

#include "silo/query_engine/operators/intersection.h"
#include "silo/query_engine/operators/selection.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

IntBetween::IntBetween(std::string column, std::optional<uint64_t> from, std::optional<uint64_t> to)
    : column(std::move(column)),
      from(from),
      to(to) {}

std::string IntBetween::toString(const silo::Database& /*database*/) const {
   const auto from_string = from.has_value() ? std::to_string(from.value()) : "unbounded";
   const auto to_string = to.has_value() ? std::to_string(to.value()) : "unbounded";

   return "[IntBetween " + from_string + " - " + to_string + "]";
}

std::unique_ptr<silo::query_engine::operators::Operator> IntBetween::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   silo::query_engine::filter_expressions::Expression::AmbiguityMode /*mode*/
) const {
   const auto& int_column = database_partition.columns.int_columns.at(column);

   std::vector<std::unique_ptr<operators::Operator>> children;
   children.emplace_back(std::make_unique<operators::Selection<int64_t>>(
      int_column.getValues(),
      operators::Selection<int64_t>::HIGHER_OR_EQUALS,
      from.value_or(0),
      database_partition.sequenceCount
   ));
   children.emplace_back(std::make_unique<operators::Selection<int64_t>>(
      int_column.getValues(),
      operators::Selection<int64_t>::LESS,
      to.value_or(INT64_MAX),
      database_partition.sequenceCount
   ));

   return std::make_unique<operators::Intersection>(
      std::move(children),
      std::vector<std::unique_ptr<operators::Operator>>(),
      database_partition.sequenceCount
   );
}

void from_json(const nlohmann::json& json, std::unique_ptr<IntBetween>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in a IntBetween expression"
   )
   CHECK_SILO_QUERY(
      json["column"].is_string(), "The field 'column' in a IntBetween expression must be a string"
   )
   CHECK_SILO_QUERY(json.contains("from"), "The field 'from' is required in IntBetween expression")
   CHECK_SILO_QUERY(
      json["from"].is_null() || json["from"].is_number_integer(),
      "The field 'from' in a IntBetween expression must be an int or null"
   )
   CHECK_SILO_QUERY(json.contains("to"), "The field 'to' is required in a IntBetween expression")
   CHECK_SILO_QUERY(
      json["to"].is_null() || json["to"].is_number_integer(),
      "The field 'to' in a IntBetween expression must be an int or null"
   )
   const std::string& column = json["column"];
   std::optional<uint64_t> value_from;
   if (json["from"].is_number_integer()) {
      value_from = json["from"].get<uint64_t>();
   }
   std::optional<uint64_t> value_to;
   if (json["to"].is_number_integer()) {
      value_to = json["to"].get<uint64_t>();
   }
   filter = std::make_unique<IntBetween>(column, value_from, value_to);
}

}  // namespace silo::query_engine::filter_expressions
