#include "silo/query_engine/filter_expressions/negation.h"

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/complement.h"

#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

Negation::Negation(std::unique_ptr<Expression> child)
    : child(std::move(child)) {}

std::string Negation::toString(const silo::Database& database) {
   return "!(" + child->toString(database) + ")";
}

std::unique_ptr<operators::Operator> Negation::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition
) const {
   auto child_operator = child->compile(database, database_partition);
   return child_operator->negate();
}

}  // namespace silo::query_engine::filter_expressions