#pragma once

#include <memory>
#include <vector>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

std::vector<CopyOnWriteBitmap> computePartitionFilters(
   const std::unique_ptr<filter::expressions::Expression>& filter,
   const storage::Table& table
);

}  // namespace silo::query_engine::operators
