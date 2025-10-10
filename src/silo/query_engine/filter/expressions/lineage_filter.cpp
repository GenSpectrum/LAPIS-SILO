#include "silo/query_engine/filter/expressions/lineage_filter.h"

#include <cctype>
#include <optional>
#include <ranges>
#include <utility>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

using silo::common::RecombinantEdgeFollowingMode;
using silo::storage::column::IndexedStringColumnPartition;

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

   if (sublineage_mode.has_value()) {
      return lineage_column.getLineageIndex()->filterIncludingSublineages(
         value_id, sublineage_mode.value()
      );
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
   return std::make_unique<operators::IndexScan>(
      CopyOnWriteBitmap{bitmap.value()}, table_partition.sequence_count
   );
}

namespace {

const std::string COLUMN_FIELD_NAME = "column";
const std::string VALUE_FIELD_NAME = "value";
const std::string INCLUDE_SUBLINEAGES_FIELD_NAME = "includeSublineages";
const std::string RECOMBINANT_FOLLOWING_MODE_FIELD_NAME = "recombinantFollowingMode";
}  // namespace

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<LineageFilter>& filter) {
   CHECK_SILO_QUERY(
      json.contains(COLUMN_FIELD_NAME),
      "The field '{}' is required in a Lineage expression",
      COLUMN_FIELD_NAME
   );
   CHECK_SILO_QUERY(
      json[COLUMN_FIELD_NAME].is_string(),
      "The field '{}' in a Lineage expression needs to be a string",
      COLUMN_FIELD_NAME
   );
   const std::string& column_name = json[COLUMN_FIELD_NAME];

   std::optional<std::string> lineage;
   CHECK_SILO_QUERY(
      json.contains(VALUE_FIELD_NAME),
      "The field '{}' is required in a Lineage expression",
      VALUE_FIELD_NAME
   );
   CHECK_SILO_QUERY(
      json[VALUE_FIELD_NAME].is_string() || json[VALUE_FIELD_NAME].is_null(),
      "The field '{}' in a Lineage expression needs to be a string or null",
      VALUE_FIELD_NAME
   );
   if (json[VALUE_FIELD_NAME].is_string()) {
      lineage = json[VALUE_FIELD_NAME].get<std::string>();
   }

   CHECK_SILO_QUERY(
      json.contains(INCLUDE_SUBLINEAGES_FIELD_NAME),
      "The field '{}' is required in a Lineage expression",
      INCLUDE_SUBLINEAGES_FIELD_NAME
   );
   CHECK_SILO_QUERY(
      json[INCLUDE_SUBLINEAGES_FIELD_NAME].is_boolean(),
      "The field '{}' in a Lineage expression needs to be a boolean",
      INCLUDE_SUBLINEAGES_FIELD_NAME
   );
   const bool include_sublineages = json[INCLUDE_SUBLINEAGES_FIELD_NAME];
   std::optional<RecombinantEdgeFollowingMode> sublineage_mode = std::nullopt;
   if (include_sublineages) {
      sublineage_mode = RecombinantEdgeFollowingMode::DO_NOT_FOLLOW;
      if (json.contains(RECOMBINANT_FOLLOWING_MODE_FIELD_NAME)) {
         static std::unordered_map<std::string, RecombinantEdgeFollowingMode>
            recombinant_following_mode_options{
               {"doNotFollow", RecombinantEdgeFollowingMode::DO_NOT_FOLLOW},
               {"followIfFullyContainedInClade",
                RecombinantEdgeFollowingMode::FOLLOW_IF_FULLY_CONTAINED_IN_CLADE},
               {"alwaysFollow", RecombinantEdgeFollowingMode::ALWAYS_FOLLOW}
            };
         CHECK_SILO_QUERY(
            json.at(RECOMBINANT_FOLLOWING_MODE_FIELD_NAME).is_string() &&
               recombinant_following_mode_options.contains(
                  json.at(RECOMBINANT_FOLLOWING_MODE_FIELD_NAME).get<std::string>()
               ),
            "The field '{}' in a Lineage expression needs to be one of: {}",
            RECOMBINANT_FOLLOWING_MODE_FIELD_NAME,
            fmt::join(recombinant_following_mode_options | std::views::keys, ",")
         );
         sublineage_mode = recombinant_following_mode_options.at(
            json.at(RECOMBINANT_FOLLOWING_MODE_FIELD_NAME).get<std::string>()
         );
      }
   }

   filter = std::make_unique<LineageFilter>(column_name, lineage, sublineage_mode);
}

}  // namespace silo::query_engine::filter::expressions
