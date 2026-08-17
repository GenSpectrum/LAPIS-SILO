#include "rhydb/query_engine/operators/compute_filter.h"

#include <memory>
#include <vector>

#include "rhydb/query_engine/copy_on_write_bitmap.h"
#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"
#include "rhydb/storage/table.h"

namespace rhydb::query_engine::operators {

using ScalarExpression = scalar_expressions::ScalarExpression;

CopyOnWriteBitmap computeFilter(
   const std::unique_ptr<ScalarExpression>& filter,
   const storage::Table& table
) {
   auto rewritten = filter->rewrite(table, ScalarExpression::AmbiguityMode::NONE);
   auto compiled = rewritten->compile(table);
   return compiled->evaluate();
}

}  // namespace rhydb::query_engine::operators
