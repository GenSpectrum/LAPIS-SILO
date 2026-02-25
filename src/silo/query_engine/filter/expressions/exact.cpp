#include "silo/query_engine/filter/expressions/exact.h"

#include <memory>
#include <string>
#include <utility>

#include <fmt/format.h>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/query_compilation_exception.h"

namespace silo::query_engine::filter::expressions {

Exact::Exact(std::unique_ptr<Expression> child)
    : child(std::move(child)) {}

std::string Exact::toString() const {
   return fmt::format("Exact ({})", child->toString());
}

std::unique_ptr<Expression> Exact::rewrite(
   const storage::Table& table,
   AmbiguityMode /*mode*/
) const {
   return child->rewrite(table, AmbiguityMode::LOWER_BOUND);
}

std::unique_ptr<operators::Operator> Exact::compile(const storage::Table& /*table*/
) const {
   throw QueryCompilationException{"Exact expression must be elimitated in query rewrite phase"};
}

}  // namespace silo::query_engine::filter::expressions
