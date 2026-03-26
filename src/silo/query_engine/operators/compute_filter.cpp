#include "silo/query_engine/operators/compute_filter.h"

#include <memory>
#include <vector>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

CopyOnWriteBitmap computeFilter(
   const std::unique_ptr<filter::expressions::Expression>& filter,
   const storage::Table& table
) {
   using Expression = filter::expressions::Expression;
   auto rewritten = filter->rewrite(table, Expression::AmbiguityMode::NONE);
   auto compiled = rewritten->compile(table);
   return compiled->evaluate();
}

}  // namespace silo::query_engine::operators
