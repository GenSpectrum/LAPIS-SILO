#pragma once

#include <memory>
#include <vector>

#include "rhydb/common/bitmap.h"
#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"
#include "rhydb/storage/table.h"

namespace rhydb::query_engine::operators {

Bitmap computeFilter(
   const std::unique_ptr<scalar_expressions::ScalarExpression>& filter,
   const storage::Table& table
);

}  // namespace rhydb::query_engine::operators
