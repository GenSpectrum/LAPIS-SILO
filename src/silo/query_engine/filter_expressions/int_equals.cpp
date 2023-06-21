#include "silo/query_engine/filter_expressions/int_equals.h"

#include <nlohmann/json.hpp>

#include "silo/database.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/index_scan.h"
#include "silo/query_engine/operators/selection.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

IntEquals::IntEquals(std::string column, uint64_t value)
    : column(std::move(column)),
      value(value) {}

std::string IntEquals::toString(const silo::Database& /*database*/) {
   return column + " = '" + std::to_string(value) + "'";
}

std::unique_ptr<silo::query_engine::operators::Operator> IntEquals::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   if (!database_partition.meta_store.int_columns.contains(column)) {
      return std::make_unique<operators::Empty>(database_partition.sequenceCount);
   }

   const auto& int_column = database_partition.meta_store.int_columns.at(column);

   return std::make_unique<operators::Selection<uint64_t>>(
      int_column.getValues(),
      operators::Selection<uint64_t>::EQUALS,
      value,
      database_partition.sequenceCount
   );
}

void from_json(const nlohmann::json& json, std::unique_ptr<IntEquals>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in an IntEquals expression"
   )
   CHECK_SILO_QUERY(
      json["column"].is_string(), "The field 'column' in an IntEquals expression must be a string"
   )
   CHECK_SILO_QUERY(
      json.contains("value"), "The field 'value' is required in an IntEquals expression"
   )
   CHECK_SILO_QUERY(
      json["value"].is_number_integer(),
      "The field 'value' in an IntEquals expression must be an integer"
   )
   const std::string& column = json["column"];
   const uint64_t& value = json["value"];
   filter = std::make_unique<IntEquals>(column, value);
}

}  // namespace silo::query_engine::filter_expressions