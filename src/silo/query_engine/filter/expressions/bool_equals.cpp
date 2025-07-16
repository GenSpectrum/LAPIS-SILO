#include "silo/query_engine/filter/expressions/bool_equals.h"

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

using silo::common::OptionalBool;
using storage::column::BoolColumnPartition;

BoolEquals::BoolEquals(std::string column_name, bool value)
    : column_name(std::move(column_name)),
      value(value) {}

std::string BoolEquals::toString() const {
   return fmt::format("{} = {}", column_name, value ? "true" : "false");
}

std::unique_ptr<silo::query_engine::filter::operators::Operator> BoolEquals::compile(
   const storage::Table& /*table*/,
   const silo::storage::TablePartition& table_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      table_partition.columns.bool_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );

   const auto& bool_column = table_partition.columns.bool_columns.at(column_name);

   return std::make_unique<operators::Selection>(
      std::make_unique<operators::CompareToValueSelection<BoolColumnPartition>>(
         bool_column, operators::Comparator::EQUALS, value
      ),
      table_partition.sequence_count
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<BoolEquals>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in an BoolEquals expression"
   );
   CHECK_SILO_QUERY(
      json["column"].is_string(), "The field 'column' in an BoolEquals expression must be a string"
   );
   CHECK_SILO_QUERY(
      json.contains("value"), "The field 'value' is required in an BoolEquals expression"
   );
   CHECK_SILO_QUERY(
      json["value"].is_boolean(), "The field 'value' in an BoolEquals expression must be a boolean"
   );
   const std::string& column_name = json["column"];
   auto value = json["value"].get<bool>();
   filter = std::make_unique<BoolEquals>(column_name, value);
}

}  // namespace silo::query_engine::filter::expressions
