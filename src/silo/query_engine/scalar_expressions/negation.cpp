#include "silo/query_engine/scalar_expressions/negation.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"

namespace silo::query_engine::scalar_expressions {

Negation::Negation(std::unique_ptr<ScalarExpression> child)
    : child(std::move(child)) {}

std::string Negation::toString() const {
   return "!(" + child->toString() + ")";
}

std::vector<schema::ColumnIdentifier> Negation::freeIUs() const {
   return child->freeIUs();
}

std::unique_ptr<ScalarExpression> Negation::rewrite(const storage::Table& table, AmbiguityMode mode)
   const {
   return std::make_unique<Negation>(child->rewrite(table, invertMode(mode)));
}

std::unique_ptr<filter::operators::Operator> Negation::compile(const storage::Table& table) const {
   return filter::operators::Operator::negate(child->compile(table));
}

}  // namespace silo::query_engine::scalar_expressions
