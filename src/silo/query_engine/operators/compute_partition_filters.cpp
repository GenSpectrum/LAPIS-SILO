#include "silo/query_engine/operators/compute_partition_filters.h"

#include <memory>
#include <vector>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

std::vector<CopyOnWriteBitmap> computePartitionFilters(
   const std::unique_ptr<filter::expressions::Expression>& filter,
   const storage::Table& table
) {
   using Expression = filter::expressions::Expression;
   std::vector<CopyOnWriteBitmap> partition_filters;
   partition_filters.reserve(table.getNumberOfPartitions());
   for (size_t i = 0; i < table.getNumberOfPartitions(); ++i) {
      auto rewritten =
         filter->rewrite(table, *table.getPartition(i), Expression::AmbiguityMode::NONE);
      auto compiled = rewritten->compile(table, *table.getPartition(i));
      partition_filters.emplace_back(compiled->evaluate());
   }
   return partition_filters;
}

}  // namespace silo::query_engine::operators
