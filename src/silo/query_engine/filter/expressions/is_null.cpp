#include "silo/query_engine/filter/expressions/is_null.h"

#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

IsNull::IsNull(std::string column_name)
    : column_name(std::move(column_name)) {}

std::string IsNull::toString() const {
   return fmt::format("{} IS NULL", column_name);
}

std::unique_ptr<Expression> IsNull::rewrite(
   const storage::Table& /*table*/,
   const storage::TablePartition& /*table_partition*/,
   Expression::AmbiguityMode /*mode*/
) const {
   return std::make_unique<IsNull>(column_name);
}

std::unique_ptr<operators::Operator> IsNull::compile(
   const storage::Table& /*table*/,
   const silo::storage::TablePartition& table_partition
) const {
   // Check bool columns - have null_bitmap
   if (table_partition.columns.bool_columns.contains(column_name)) {
      const auto& column = table_partition.columns.bool_columns.at(column_name);
      return std::make_unique<operators::IndexScan>(
         CopyOnWriteBitmap{&column.null_bitmap}, table_partition.sequence_count
      );
   }

   // Check int columns - have null_bitmap
   if (table_partition.columns.int_columns.contains(column_name)) {
      const auto& column = table_partition.columns.int_columns.at(column_name);
      return std::make_unique<operators::IndexScan>(
         CopyOnWriteBitmap{&column.null_bitmap}, table_partition.sequence_count
      );
   }

   // Check float columns - have null_bitmap
   if (table_partition.columns.float_columns.contains(column_name)) {
      const auto& column = table_partition.columns.float_columns.at(column_name);
      return std::make_unique<operators::IndexScan>(
         CopyOnWriteBitmap{&column.null_bitmap}, table_partition.sequence_count
      );
   }

   // Check string columns - have null_bitmap
   if (table_partition.columns.string_columns.contains(column_name)) {
      const auto& column = table_partition.columns.string_columns.at(column_name);
      return std::make_unique<operators::IndexScan>(
         CopyOnWriteBitmap{&column.null_bitmap}, table_partition.sequence_count
      );
   }

   // Check indexed string columns - use filter(std::nullopt) for null bitmap
   if (table_partition.columns.indexed_string_columns.contains(column_name)) {
      const auto& column = table_partition.columns.indexed_string_columns.at(column_name);
      const auto bitmap = column.filter(std::nullopt);
      if (bitmap == std::nullopt || bitmap.value()->isEmpty()) {
         return std::make_unique<operators::Empty>(table_partition.sequence_count);
      }
      return std::make_unique<operators::IndexScan>(
         CopyOnWriteBitmap{bitmap.value()}, table_partition.sequence_count
      );
   }

   // Check date columns - no null_bitmap, use Selection with isNull check
   if (table_partition.columns.date_columns.contains(column_name)) {
      const auto& column = table_partition.columns.date_columns.at(column_name);
      return std::make_unique<operators::IndexScan>(
         CopyOnWriteBitmap{&column.null_bitmap}, table_partition.sequence_count
      );
   }

   // TODO(#547) also check NULL for aligned sequences and unaligned sequences
   throw IllegalQueryException(
      "The database does not contain a column '{}' that supports null checks", column_name
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<IsNull>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in an IsNull expression"
   );
   CHECK_SILO_QUERY(
      json["column"].is_string(), "The field 'column' in an IsNull expression must be a string"
   );
   const std::string& column_name = json["column"];
   filter = std::make_unique<IsNull>(column_name);
}

}  // namespace silo::query_engine::filter::expressions
