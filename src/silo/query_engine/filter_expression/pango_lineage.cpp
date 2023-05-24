#include "silo/query_engine/filter_expressions/pango_lineage.h"

#include "silo/query_engine/operators/index_scan.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

PangoLineage::PangoLineage(uint32_t lineage_key, bool include_sublineages)
    : lineage_key(lineage_key),
      include_sublineages(include_sublineages) {}

std::string PangoLineage::toString(const silo::Database& /*database*/) {
   std::string res = std::to_string(lineage_key);
   if (include_sublineages) {
      res += "*";
   }
   return res;
}

std::unique_ptr<silo::query_engine::operators::Operator> PangoLineage::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition
) const {
   if (include_sublineages) {
      return std::make_unique<operators::IndexScan>(
         &database_partition.meta_store.sublineage_bitmaps[lineage_key]
      );
   }
   return std::make_unique<operators::IndexScan>(
      &database_partition.meta_store.lineage_bitmaps[lineage_key]
   );
}

}  // namespace silo::query_engine::filter_expressions