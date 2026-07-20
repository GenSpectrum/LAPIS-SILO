#include "silo/query_engine/scalar_expressions/scalar_expression.h"

namespace silo::query_engine::scalar_expressions {

ScalarExpression::ScalarExpression() = default;

ScalarExpression::AmbiguityMode invertMode(ScalarExpression::AmbiguityMode mode) {
   if (mode == ScalarExpression::UPPER_BOUND) {
      return ScalarExpression::LOWER_BOUND;
   }
   if (mode == ScalarExpression::LOWER_BOUND) {
      return ScalarExpression::UPPER_BOUND;
   }
   return mode;
}

}  // namespace silo::query_engine::scalar_expressions
