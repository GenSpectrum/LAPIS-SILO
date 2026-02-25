#include "silo/query_engine/filter/expressions/expression.h"

namespace silo::query_engine::filter::expressions {

Expression::Expression() = default;

Expression::AmbiguityMode invertMode(Expression::AmbiguityMode mode) {
   if (mode == Expression::UPPER_BOUND) {
      return Expression::LOWER_BOUND;
   }
   if (mode == Expression::LOWER_BOUND) {
      return Expression::UPPER_BOUND;
   }
   return mode;
}

}  // namespace silo::query_engine::filter::expressions
