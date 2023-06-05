#include <utility>

#include "silo/query_engine/filter_expressions/pango_lineage_filter.h"

#include "silo/database.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/index_scan.h"
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

std::string PangoLineageFilter::toString(const silo::Database& /*database*/) {
   std::string res = lineage;
   if (include_sublineages) {
      res += "*";
   }
   return res;
}

std::unique_ptr<silo::query_engine::operators::Operator> PangoLineageFilter::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition
) const {
   if (!database_partition.meta_store.pango_lineage_columns.contains(column)) {
      return std::make_unique<operators::Empty>();
   }

   std::string lineage_copy = lineage;
   std::transform(lineage_copy.begin(), lineage_copy.end(), lineage_copy.begin(), ::toupper);
   const auto resolved_lineage = database.getAliasKey().resolvePangoLineageAlias(lineage_copy);

   const auto& pango_lineage_column =
      database_partition.meta_store.pango_lineage_columns.at(column);
   const auto& bitmap = include_sublineages
                           ? pango_lineage_column.filterIncludingSublineages({resolved_lineage})
                           : pango_lineage_column.filter({resolved_lineage});

   return std::make_unique<operators::IndexScan>(new roaring::Roaring(bitmap));
}

}  // namespace silo::query_engine::filter_expressions