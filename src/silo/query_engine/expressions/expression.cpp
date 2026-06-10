#include "silo/query_engine/expressions/expression.h"

#include <cstddef>

namespace silo::query_engine::expressions {

Expression::Expression() = default;

bool expressionVectorsEqual(const ExpressionVector& lhs, const ExpressionVector& rhs) {
   if (lhs.size() != rhs.size()) {
      return false;
   }
   for (size_t i = 0; i < lhs.size(); ++i) {
      if (!(*lhs[i] == *rhs[i])) {
         return false;
      }
   }
   return true;
}

Expression::AmbiguityMode invertMode(Expression::AmbiguityMode mode) {
   if (mode == Expression::UPPER_BOUND) {
      return Expression::LOWER_BOUND;
   }
   if (mode == Expression::LOWER_BOUND) {
      return Expression::UPPER_BOUND;
   }
   return mode;
}

}  // namespace silo::query_engine::expressions
