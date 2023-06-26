#include "silo/query_engine/filter_expressions/pango_lineage_filter.h"

#include <utility>

#include <nlohmann/json.hpp>

#include "silo/database.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/index_scan.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/column/pango_lineage_column.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

PangoLineageFilter::PangoLineageFilter(
   std::string column,
   std::string lineage,
   bool include_sublineages
)
    : column(std::move(column)),
      lineage(std::move(lineage)),
      include_sublineages(include_sublineages) {}

std::string PangoLineageFilter::toString(const silo::Database& /*database*/) const {
   std::string res = lineage;
   if (include_sublineages) {
      res += "*";
   }
   return res;
}

std::unique_ptr<silo::query_engine::operators::Operator> PangoLineageFilter::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   AmbiguityMode /*mode*/
) const {
   if (!database_partition.meta_store.pango_lineage_columns.contains(column)) {
      return std::make_unique<operators::Empty>(database_partition.sequenceCount);
   }

   std::string lineage_copy = lineage;
   std::transform(lineage_copy.begin(), lineage_copy.end(), lineage_copy.begin(), ::toupper);
   const auto resolved_lineage = database.getAliasKey().resolvePangoLineageAlias(lineage_copy);

   const auto& pango_lineage_column =
      database_partition.meta_store.pango_lineage_columns.at(column);
   const auto& bitmap = include_sublineages
                           ? pango_lineage_column.filterIncludingSublineages({resolved_lineage})
                           : pango_lineage_column.filter({resolved_lineage});

   return std::make_unique<operators::IndexScan>(
      new roaring::Roaring(bitmap), database_partition.sequenceCount
   );
}

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
