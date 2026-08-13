#pragma once

#include <memory>
#include <vector>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"
#include "silo/storage/table.h"

namespace rhydb::query_engine::operators {

CopyOnWriteBitmap computeFilter(
   const std::unique_ptr<scalar_expressions::ScalarExpression>& filter,
   const storage::Table& table
);

}  // namespace rhydb::query_engine::operators
