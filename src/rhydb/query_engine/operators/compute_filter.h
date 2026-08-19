#pragma once

#include <memory>
#include <vector>

#include "rhydb/query_engine/copy_on_write_bitmap.h"
#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"
#include "rhydb/storage/table.h"

namespace rhydb::query_engine::operators {

CopyOnWriteBitmap computeFilter(
   const std::unique_ptr<scalar_expressions::ScalarExpression>& filter,
   const storage::Table& table
);

}  // namespace rhydb::query_engine::operators
