#include "silo/query_engine/filter/expressions/maybe.h"

#include <memory>
#include <string>
#include <utility>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/query_compilation_exception.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

Maybe::Maybe(std::unique_ptr<Expression> child)
    : child(std::move(child)) {}

std::string Maybe::toString() const {
   return "Maybe (" + child->toString() + ")";
}

std::unique_ptr<Expression> Maybe::rewrite(
   const storage::Table& table,
   const storage::TablePartition& table_partition,
   AmbiguityMode /*mode*/
) const {
   return child->rewrite(table, table_partition, AmbiguityMode::UPPER_BOUND);
}

std::unique_ptr<operators::Operator> Maybe::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& /*table_partition*/
) const {
   throw QueryCompilationException{"Maybe expression must be elimitated in query rewrite phase"};
}

}  // namespace silo::query_engine::filter::expressions
