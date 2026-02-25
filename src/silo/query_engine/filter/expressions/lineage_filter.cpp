#include "silo/query_engine/filter/expressions/lineage_filter.h"

#include <cctype>
#include <optional>
#include <ranges>
#include <utility>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"

namespace silo::query_engine::filter::expressions {

using silo::common::RecombinantEdgeFollowingMode;
using silo::storage::column::IndexedStringColumn;

LineageFilter::LineageFilter(
   std::string column_name,
   std::optional<std::string> lineage,
   std::optional<RecombinantEdgeFollowingMode> sublineage_mode
)
    : column_name(std::move(column_name)),
      lineage(std::move(lineage)),
      sublineage_mode(sublineage_mode) {}

std::string LineageFilter::toString() const {
   if (!lineage.has_value()) {
      return "NULL";
   }
   if (sublineage_mode.has_value()) {
      return "'" + lineage.value() + "*'";
   }
   return "'" + lineage.value() + "'";
}

std::optional<const roaring::Roaring*> LineageFilter::getBitmapForValue(
   const IndexedStringColumn& lineage_column
) const {
   if (lineage == std::nullopt) {
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

   if (sublineage_mode.has_value()) {
      return lineage_column.getLineageIndex()->filterIncludingSublineages(
         value_id, sublineage_mode.value()
      );
   }
   return lineage_column.getLineageIndex()->filterExcludingSublineages(value_id);
}

std::unique_ptr<Expression> LineageFilter::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<LineageFilter>(column_name, lineage, sublineage_mode);
}

std::unique_ptr<operators::Operator> LineageFilter::compile(const storage::Table& table) const {
   CHECK_SILO_QUERY(
      table.columns.indexed_string_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );
   CHECK_SILO_QUERY(
      table.columns.indexed_string_columns.at(column_name).getLineageIndex().has_value(),
      "The database does not contain a lineage index for the column '{}'",
      column_name
   );

   const auto& lineage_column = table.columns.indexed_string_columns.at(column_name);
   std::optional<const roaring::Roaring*> bitmap = getBitmapForValue(lineage_column);

   if (bitmap == std::nullopt) {
      return std::make_unique<operators::Empty>(table.sequence_count);
   }
   return std::make_unique<operators::IndexScan>(
      CopyOnWriteBitmap{bitmap.value()}, table.sequence_count
   );
}

}  // namespace silo::query_engine::filter::expressions
