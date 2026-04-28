#include "silo/query_engine/filter/expressions/negation.h"

#include <memory>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"

namespace silo::query_engine::filter::expressions {

Negation::Negation(std::unique_ptr<Expression> child)
    : child(std::move(child)) {}

std::string Negation::toString() const {
   return "!(" + child->toString() + ")";
}

std::unique_ptr<Expression> Negation::rewrite(const storage::Table& table, AmbiguityMode mode)
   const {
   return std::make_unique<Negation>(child->rewrite(table, invertMode(mode)));
}

std::unique_ptr<operators::Operator> Negation::compile(const storage::Table& table) const {
   return operators::Operator::negate(child->compile(table));
}

}  // namespace silo::query_engine::filter::expressions
