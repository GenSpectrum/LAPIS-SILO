#include "silo/query_engine/filter/expressions/float_equals.h"

#include <cmath>
#include <memory>
#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/storage/table_partition.h"

using silo::storage::column::FloatColumnPartition;

namespace silo::query_engine::filter::expressions {

FloatEquals::FloatEquals(std::string column_name, std::optional<double> value)
    : column_name(std::move(column_name)),
      value(value) {}

std::string FloatEquals::toString() const {
   if (value.has_value()) {
      return fmt::format("{} = '{}'", column_name, value.value());
   }
   return fmt::format("{} IS NULL", column_name);
}

std::unique_ptr<Expression> FloatEquals::rewrite(
   const storage::Table& /*table*/,
   const storage::TablePartition& /*table_partition*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<FloatEquals>(column_name, value);
}

std::unique_ptr<operators::Operator> FloatEquals::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition
) const {
   CHECK_SILO_QUERY(
      table_partition.columns.float_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );

   const auto& float_column = table_partition.columns.float_columns.at(column_name);

   if (value.has_value()) {
      return std::make_unique<operators::Selection>(
         std::make_unique<operators::CompareToValueSelection<FloatColumnPartition>>(
            float_column, operators::Comparator::EQUALS, value.value()
         ),
         table_partition.sequence_count
      );
   }
   return std::make_unique<operators::IndexScan>(
      CopyOnWriteBitmap{&float_column.null_bitmap}, table_partition.sequence_count
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FloatEquals>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in a FloatEquals expression"
   );
   CHECK_SILO_QUERY(
      json["column"].is_string(), "The field 'column' in a FloatEquals expression must be a string"
   );
   CHECK_SILO_QUERY(
      json.contains("value"), "The field 'value' is required in a FloatEquals expression"
   );
   CHECK_SILO_QUERY(
      json["value"].is_number_float() || json["value"].is_null(),
      "The field 'value' in a FloatEquals expression must be a float or null"
   );
   const std::string& column_name = json["column"];
   std::optional<double> value;
   if (!json["value"].is_null()) {
      value = json["value"].get<double>();
   }
   filter = std::make_unique<FloatEquals>(column_name, value);
}

}  // namespace silo::query_engine::filter::expressions
