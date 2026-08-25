#include "rhydb/query_engine/exec_node/scalar_to_arrow_expression.h"

#include <arrow/compute/api.h>

namespace rhydb::query_engine::exec_node {

using scalar_expressions::ScalarExpression;

arrow::Result<arrow::compute::Expression> scalarToArrowExpression(const ScalarExpression& expression
) {
   return expression.toArrowExpression();
}

}  // namespace rhydb::query_engine::exec_node
