#include "silo/query_engine/filter/expressions/is_null.h"

#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/storage/column/column_type_visitor.h"
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
   const storage::Table& table,
   const silo::storage::TablePartition& table_partition
) const {
   const auto& maybe_target_column = table.schema.getColumn(column_name);
   CHECK_SILO_QUERY(
      maybe_target_column.has_value(),
      "The column '{}' is not contained in the database",
      column_name
   );
   auto target_column = maybe_target_column.value();

   return silo::storage::column::visit(target_column.type, [&]<storage::column::Column Column>() {
      const auto& column = table_partition.columns.getColumns<Column>().at(column_name);
      return std::make_unique<operators::IndexScan>(
         CopyOnWriteBitmap{&column.null_bitmap}, table_partition.sequence_count
      );
   });
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
