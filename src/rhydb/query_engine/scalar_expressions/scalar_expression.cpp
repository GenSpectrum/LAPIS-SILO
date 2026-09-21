#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"

#include <arrow/compute/api.h>

namespace rhydb::query_engine::scalar_expressions {

ScalarExpression::ScalarExpression() = default;

arrow::Result<arrow::compute::Expression> ScalarExpression::toArrowExpression() const {
   return arrow::Status::NotImplemented(
      "the scalar expression ", toString(), " has no Arrow compute translation"
   );
}

ScalarExpression::AmbiguityMode invertMode(ScalarExpression::AmbiguityMode mode) {
   if (mode == ScalarExpression::UPPER_BOUND) {
      return ScalarExpression::LOWER_BOUND;
   }
   if (mode == ScalarExpression::LOWER_BOUND) {
      return ScalarExpression::UPPER_BOUND;
   }
   return mode;
}

}  // namespace rhydb::query_engine::scalar_expressions
