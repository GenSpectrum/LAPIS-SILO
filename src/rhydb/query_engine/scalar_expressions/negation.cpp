#include "rhydb/query_engine/scalar_expressions/negation.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <arrow/compute/api.h>
#include <nlohmann/json.hpp>

#include "rhydb/query_engine/filter/operators/operator.h"
#include "rhydb/query_engine/illegal_query_exception.h"
#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"

namespace rhydb::query_engine::scalar_expressions {

Negation::Negation(std::unique_ptr<ScalarExpression> child)
    : child(std::move(child)) {}

std::string Negation::toString() const {
   return "!(" + child->toString() + ")";
}

std::vector<schema::ColumnIdentifier> Negation::freeIUs() const {
   return child->freeIUs();
}

arrow::Result<arrow::compute::Expression> Negation::toArrowExpression() const {
   ARROW_ASSIGN_OR_RAISE(auto child_expression, child->toArrowExpression());
   // SILO negation is a set complement over all rows, not SQL three-valued logic: `!(x)` keeps
   // every row that is not in `x`'s result set, i.e. where `x` is false OR null. A bare Arrow
   // `invert` would follow 3VL (`invert(null) = null`) and drop null rows, diverging from the
   // bitmap filter path. `coalesce(invert(x), true)` maps a null result back to true so those rows
   // are kept.
   return arrow::compute::call(
      "coalesce", {arrow::compute::not_(child_expression), arrow::compute::literal(true)}
   );
}

std::unique_ptr<ScalarExpression> Negation::rewrite(const storage::Table& table, AmbiguityMode mode)
   const {
   return std::make_unique<Negation>(child->rewrite(table, invertMode(mode)));
}

std::unique_ptr<filter::operators::Operator> Negation::compile(const storage::Table& table) const {
   return filter::operators::Operator::negate(child->compile(table));
}

}  // namespace rhydb::query_engine::scalar_expressions
