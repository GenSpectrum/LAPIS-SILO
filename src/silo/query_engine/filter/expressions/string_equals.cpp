#include "silo/query_engine/filter/expressions/string_equals.h"
#include <fmt/format.h>

#include <optional>
#include <utility>

#include <nlohmann/json.hpp>

#include "silo/common/panic.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/expressions/is_null.h"
#include "silo/query_engine/filter/expressions/string_in_set.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"
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

std::unique_ptr<Expression> StringEquals::rewrite(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition,
   AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      table_partition.columns.string_columns.contains(column_name) ||
         table_partition.columns.indexed_string_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );

   if (value == std::nullopt) {
      return std::make_unique<IsNull>(column_name);
   }

   // We do not change expressions for IndexedStringColumn
   if (table_partition.columns.indexed_string_columns.contains(column_name)) {
      return std::make_unique<StringEquals>(column_name, value);
   }

   SILO_ASSERT(table_partition.columns.string_columns.contains(column_name));

   return std::make_unique<StringInSet>(
      column_name, std::unordered_set<std::string>{value.value()}
   );
}

std::unique_ptr<operators::Operator> StringEquals::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition
) const {
   // If it was a StringColumn it should have been rewritten
   SILO_ASSERT(table_partition.columns.indexed_string_columns.contains(column_name));
   const auto& string_column = table_partition.columns.indexed_string_columns.at(column_name);
   const auto bitmap = string_column.filter(value);

   if (bitmap == std::nullopt || bitmap.value()->isEmpty()) {
      return std::make_unique<operators::Empty>(table_partition.sequence_count);
   }
   return std::make_unique<operators::IndexScan>(
      CopyOnWriteBitmap{bitmap.value()}, table_partition.sequence_count
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
