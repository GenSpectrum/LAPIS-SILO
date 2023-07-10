#include "silo/query_engine/filter_expressions/insertion_contains.h"

#include <map>
#include <unordered_map>
#include <utility>

#include <nlohmann/json.hpp>

#include "silo/common/string.h"
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/column/insertion_column.h"
#include "silo/storage/column_group.h"
#include "silo/storage/database_partition.h"

namespace silo {
class Database;
namespace query_engine::operators {
class Operator;
}  // namespace query_engine::operators
}  // namespace silo

namespace silo::query_engine::filter_expressions {

InsertionContains::InsertionContains(std::string column, std::string value)
    : column_name(std::move(column)),
      value(std::move(value)) {}

std::string InsertionContains::toString(const silo::Database& /*database*/) const {
   return column_name + " has insertion '" + value + "'";
}

std::unique_ptr<silo::query_engine::operators::Operator> InsertionContains::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      database_partition.columns.insertion_columns.contains(column_name),
      "The insertion column '" + column_name + "' does not exist."
   )

   // NOLINTNEXTLINE(clang-diagnostic-unused-variable)
   const storage::column::InsertionColumnPartition& column =
      database_partition.columns.insertion_columns.at(column_name);

   // TODO(#164) correctly return an operator tree that filters the given partition to contain
   // the given insertion pattern
   return std::make_unique<operators::Empty>(database_partition.sequenceCount);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<InsertionContains>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in an InsertionContains expression"
   )
   CHECK_SILO_QUERY(
      json["column"].is_string(),
      "The field 'column' in an InsertionContains expression needs to be a string"
   )
   CHECK_SILO_QUERY(
      json.contains("value"), "The field 'value' is required in an InsertionContains expression"
   )
   CHECK_SILO_QUERY(
      json["value"].is_string() || json["value"].is_null(),
      "The field 'value' in an InsertionContains expression needs to be a string or null"
   )
   // TODO(#164) maybe validate the value field
   const std::string& column_name = json["column"];
   const std::string& value = json["value"].is_null() ? "" : json["value"].get<std::string>();
   filter = std::make_unique<InsertionContains>(column_name, value);
}

}  // namespace silo::query_engine::filter_expressions