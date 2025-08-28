#include "silo/query_engine/filter/expressions/int_equals.h"

#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/storage/table_partition.h"

using silo::storage::column::IntColumnPartition;

namespace silo::query_engine::filter::expressions {

IntEquals::IntEquals(std::string column_name, std::optional<uint32_t> value)
    : column_name(std::move(column_name)),
      value(value) {}

std::string IntEquals::toString() const {
   if (value.has_value()) {
      return fmt::format("{} = '{}'", column_name, value.value());
   }
   return fmt::format("{} IS NULL", column_name);
}

std::unique_ptr<silo::query_engine::filter::operators::Operator> IntEquals::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      table_partition.columns.int_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );

   const auto& int_column = table_partition.columns.int_columns.at(column_name);

   if (value.has_value()) {
      return std::make_unique<operators::Selection>(
         std::make_unique<operators::CompareToValueSelection<IntColumnPartition>>(
            int_column, operators::Comparator::EQUALS, value.value()
         ),
         table_partition.sequence_count
      );
   }
   return std::make_unique<operators::IndexScan>(
      &int_column.null_bitmap, table_partition.sequence_count
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
      "The field 'value' in an IntEquals expression must be an integer in [-2147483648; "
      "2147483647] or null"
   );
   const std::string& column = json["column"];
   std::optional<int32_t> value;
   if (!json["value"].is_null()) {
      value = json["value"].get<int32_t>();
   }
   filter = std::make_unique<IntEquals>(column, value);
}

}  // namespace silo::query_engine::filter::expressions
