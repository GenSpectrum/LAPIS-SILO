#include "silo/query_engine/operators/compute_filter.h"

#include <memory>
#include <vector>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

using ScalarExpression = scalar_expressions::ScalarExpression;

CopyOnWriteBitmap computeFilter(
   const std::unique_ptr<ScalarExpression>& filter,
   const storage::Table& table
) {
   auto rewritten = filter->rewrite(table, ScalarExpression::AmbiguityMode::NONE);
   auto compiled = rewritten->compile(table);
   return compiled->evaluate();
}

}  // namespace silo::query_engine::operators
