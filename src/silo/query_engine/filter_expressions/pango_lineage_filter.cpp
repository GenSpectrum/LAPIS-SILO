#include "silo/query_engine/filter_expressions/pango_lineage_filter.h"

#include <cctype>
#include <optional>
#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/index_scan.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::operators {
class Operator;
}  // namespace silo::query_engine::operators

namespace silo::query_engine::filter_expressions {

PangoLineageFilter::PangoLineageFilter(
   std::string column,
   std::string lineage,
   bool include_sublineages
)
    : column(std::move(column)),
      lineage(std::move(lineage)),
      include_sublineages(include_sublineages) {}

std::string PangoLineageFilter::toString() const {
   std::string res = lineage;
   if (include_sublineages) {
      res += "*";
   }
   return res;
}

std::unique_ptr<silo::query_engine::operators::Operator> PangoLineageFilter::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      database_partition.columns.pango_lineage_columns.contains(column),
      fmt::format("the database does not contain the column {}", column)
   );

   std::string lineage_all_upper = lineage;
   std::transform(
      lineage_all_upper.begin(), lineage_all_upper.end(), lineage_all_upper.begin(), ::toupper
   );

   const auto& pango_lineage_column = database_partition.columns.pango_lineage_columns.at(column);
   const auto& bitmap = include_sublineages
                           ? pango_lineage_column.filterIncludingSublineages({lineage_all_upper})
                           : pango_lineage_column.filter({lineage_all_upper});
   if (bitmap == std::nullopt) {
      return std::make_unique<operators::Empty>(database_partition.sequence_count);
   }
   return std::make_unique<operators::IndexScan>(bitmap.value(), database_partition.sequence_count);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<PangoLineageFilter>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in a PangoLineage expression"
   )
   CHECK_SILO_QUERY(
      json["column"].is_string(),
      "The field 'column' in a PangoLineage expression needs to be a string"
   )
   CHECK_SILO_QUERY(
      json.contains("value"), "The field 'value' is required in a PangoLineage expression"
   )
   CHECK_SILO_QUERY(
      json["value"].is_string(),
      "The field 'value' in a PangoLineage expression needs to be a string"
   )
   CHECK_SILO_QUERY(
      json.contains("includeSublineages"),
      "The field 'includeSublineages' is required in a PangoLineage expression"
   )
   CHECK_SILO_QUERY(
      json["includeSublineages"].is_boolean(),
      "The field 'includeSublineages' in a PangoLineage expression needs to be a boolean"
   )
   const std::string& column = json["column"];
   const std::string& lineage = json["value"];
   const bool include_sublineages = json["includeSublineages"];
   filter = std::make_unique<PangoLineageFilter>(column, lineage, include_sublineages);
}

}  // namespace silo::query_engine::filter_expressions
