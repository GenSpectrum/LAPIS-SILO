#include "silo/query_engine/filter/expressions/string_equals.h"
#include <fmt/format.h>

#include <optional>
#include <utility>

#include <nlohmann/json.hpp>

#include "silo/common/panic.h"
#include "silo/common/string.h"
#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

StringEquals::StringEquals(std::string column_name, std::string value)
    : column_name(std::move(column_name)),
      value(std::move(value)) {}

std::string StringEquals::toString() const {
   return fmt::format("{} = '{}'", column_name, value);
}

std::unique_ptr<silo::query_engine::filter::operators::Operator> StringEquals::compile(
   const Database& /*database*/,
   const storage::TablePartition& database_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      database_partition.columns.string_columns.contains(column_name) ||
         database_partition.columns.indexed_string_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );

   if (database_partition.columns.indexed_string_columns.contains(column_name)) {
      const auto& string_column = database_partition.columns.indexed_string_columns.at(column_name);
      const auto bitmap = string_column.filter(value);

      if (bitmap == std::nullopt || bitmap.value()->isEmpty()) {
         return std::make_unique<operators::Empty>(database_partition.sequence_count);
      }
      return std::make_unique<operators::IndexScan>(
         bitmap.value(), database_partition.sequence_count
      );
   }
   SILO_ASSERT(database_partition.columns.string_columns.contains(column_name));
   const auto& string_column = database_partition.columns.string_columns.at(column_name);
   const auto& embedded_string = string_column.embedString(value);
   if (embedded_string.has_value()) {
      return std::make_unique<operators::Selection>(
         std::make_unique<operators::CompareToValueSelection<common::SiloString>>(
            string_column.getValues(), operators::Comparator::EQUALS, embedded_string.value()
         ),
         database_partition.sequence_count
      );
   }
   return std::make_unique<operators::Empty>(database_partition.sequence_count);
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
   const std::string& value = json["value"].is_null() ? "" : json["value"].get<std::string>();
   filter = std::make_unique<StringEquals>(column_name, value);
}

}  // namespace silo::query_engine::filter::expressions
