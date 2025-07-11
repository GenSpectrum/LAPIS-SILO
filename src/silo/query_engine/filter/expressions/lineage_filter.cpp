#include "silo/query_engine/filter/expressions/lineage_filter.h"

#include <cctype>
#include <optional>
#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

using silo::storage::column::IndexedStringColumnPartition;

LineageFilter::LineageFilter(
   std::string column_name,
   std::optional<std::string> lineage,
   bool include_sublineages
)
    : column_name(std::move(column_name)),
      lineage(std::move(lineage)),
      include_sublineages(include_sublineages) {}

std::string LineageFilter::toString() const {
   if (!lineage.has_value()) {
      return "NULL";
   }
   if (include_sublineages) {
      return "'" + lineage.value() + "*'";
   }
   return "'" + lineage.value() + "'";
}

std::optional<const roaring::Roaring*> LineageFilter::getBitmapForValue(
   const IndexedStringColumnPartition& lineage_column
) const {
   if (!lineage) {
      return lineage_column.filter(std::nullopt);
   }

   const auto value_id_opt = lineage_column.getValueId(lineage.value());

   CHECK_SILO_QUERY(
      value_id_opt.has_value(),
      "The lineage '{}' is not a valid lineage for column '{}'.",
      lineage.value(),
      column_name
   );

   const Idx value_id = value_id_opt.value();

   if (include_sublineages) {
      return lineage_column.getLineageIndex()->filterIncludingSublineages(value_id);
   }
   return lineage_column.getLineageIndex()->filterExcludingSublineages(value_id);
}

std::unique_ptr<silo::query_engine::filter::operators::Operator> LineageFilter::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition,
   AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      table_partition.columns.indexed_string_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );
   CHECK_SILO_QUERY(
      table_partition.columns.indexed_string_columns.at(column_name).getLineageIndex().has_value(),
      "The database does not contain a lineage index for the column '{}'",
      column_name
   );

   const auto& lineage_column = table_partition.columns.indexed_string_columns.at(column_name);
   std::optional<const roaring::Roaring*> bitmap = getBitmapForValue(lineage_column);

   if (bitmap == std::nullopt) {
      return std::make_unique<operators::Empty>(table_partition.sequence_count);
   }
   return std::make_unique<operators::IndexScan>(bitmap.value(), table_partition.sequence_count);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<LineageFilter>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in a Lineage expression"
   );
   CHECK_SILO_QUERY(
      json["column"].is_string(), "The field 'column' in a Lineage expression needs to be a string"
   );
   CHECK_SILO_QUERY(
      json.contains("value"), "The field 'value' is required in a Lineage expression"
   );
   CHECK_SILO_QUERY(
      json["value"].is_string() || json["value"].is_null(),
      "The field 'value' in a Lineage expression needs to be a string or null"
   );
   CHECK_SILO_QUERY(
      json.contains("includeSublineages"),
      "The field 'includeSublineages' is required in a Lineage expression"
   );
   CHECK_SILO_QUERY(
      json["includeSublineages"].is_boolean(),
      "The field 'includeSublineages' in a Lineage expression needs to be a boolean"
   );
   const std::string& column_name = json["column"];
   std::optional<std::string> lineage;
   if (json["value"].is_string()) {
      lineage = json["value"].get<std::string>();
   }
   const bool include_sublineages = json["includeSublineages"];
   filter = std::make_unique<LineageFilter>(column_name, lineage, include_sublineages);
}

}  // namespace silo::query_engine::filter::expressions
