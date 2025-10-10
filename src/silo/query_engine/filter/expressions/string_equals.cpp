#include "silo/query_engine/filter/expressions/string_equals.h"
#include <fmt/format.h>

#include <optional>
#include <utility>

#include <nlohmann/json.hpp>

#include "silo/common/german_string.h"
#include "silo/common/panic.h"
#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

using storage::column::StringColumnPartition;

StringEquals::StringEquals(std::string column_name, std::optional<std::string> value)
    : column_name(std::move(column_name)),
      value(std::move(value)) {}

std::string StringEquals::toString() const {
   if (value.has_value()) {
      return fmt::format("{} = '{}'", column_name, value.value());
   }
   return fmt::format("{} IS NULL", column_name);
}

std::unique_ptr<silo::query_engine::filter::operators::Operator> StringEquals::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      table_partition.columns.string_columns.contains(column_name) ||
         table_partition.columns.indexed_string_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );

   if (table_partition.columns.indexed_string_columns.contains(column_name)) {
      const auto& string_column = table_partition.columns.indexed_string_columns.at(column_name);
      const auto bitmap = string_column.filter(value);

      if (bitmap == std::nullopt || bitmap.value()->isEmpty()) {
         return std::make_unique<operators::Empty>(table_partition.sequence_count);
      }
      return std::make_unique<operators::IndexScan>(
         CopyOnWriteBitmap{bitmap.value()}, table_partition.sequence_count
      );
   }
   SILO_ASSERT(table_partition.columns.string_columns.contains(column_name));
   const auto& string_column = table_partition.columns.string_columns.at(column_name);
   if (value.has_value()) {
      return std::make_unique<operators::Selection>(
         std::make_unique<operators::CompareToValueSelection<StringColumnPartition>>(
            string_column, operators::Comparator::EQUALS, value.value()
         ),
         table_partition.sequence_count
      );
   }
   return std::make_unique<operators::IndexScan>(
      CopyOnWriteBitmap{&string_column.null_bitmap}, table_partition.sequence_count
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<StringEquals>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in an StringEquals expression"
   );
   CHECK_SILO_QUERY(
      json["column"].is_string(),
      "The field 'column' in an StringEquals expression needs to be a string"
   );
   CHECK_SILO_QUERY(
      json.contains("value"), "The field 'value' is required in an StringEquals expression"
   );
   CHECK_SILO_QUERY(
      json["value"].is_string() || json["value"].is_null(),
      "The field 'value' in an StringEquals expression needs to be a string or null"
   );
   const std::string& column_name = json["column"];
   std::optional<std::string> value;
   if (!json["value"].is_null()) {
      value = json["value"].get<std::string>();
   }
   filter = std::make_unique<StringEquals>(column_name, value);
}

}  // namespace silo::query_engine::filter::expressions
