#include "silo/query_engine/filter_expressions/pango_lineage.h"

#include "silo/database.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/index_scan.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

PangoLineage::PangoLineage(std::string lineage, bool include_sublineages)
    : lineage(std::move(lineage)),
      include_sublineages(include_sublineages) {}

std::string PangoLineage::toString(const silo::Database& /*database*/) {
   std::string res = lineage;
   if (include_sublineages) {
      res += "*";
   }
   return res;
}

std::unique_ptr<silo::query_engine::operators::Operator> PangoLineage::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition
) const {
   std::string resolved_lineage = lineage;
   std::transform(
      resolved_lineage.begin(), resolved_lineage.end(), resolved_lineage.begin(), ::toupper
   );
   resolved_lineage = database.getAliasKey().resolvePangoLineageAlias(resolved_lineage);
   const std::optional<uint32_t> lineage_key =
      database.dict->getPangoLineageIdInLookup(resolved_lineage);
   if (!lineage_key.has_value()) {
      return std::make_unique<operators::Empty>();
   }
   if (include_sublineages) {
      return std::make_unique<operators::IndexScan>(
         &database_partition.meta_store.sublineage_bitmaps[lineage_key.value()]
      );
   }
   return std::make_unique<operators::IndexScan>(
      &database_partition.meta_store.lineage_bitmaps[lineage_key.value()]
   );
}

}  // namespace silo::query_engine::filter_expressions