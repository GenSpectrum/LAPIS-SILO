#include "silo/query_engine/filter_expressions/false.h"

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/empty.h"

#include "silo/storage/database_partition.h"

namespace operators = silo::query_engine::operators;

namespace silo::query_engine::filter_expressions {

False::False() = default;

std::string False::toString(const silo::Database& database) {
   return "False";
}

std::unique_ptr<silo::query_engine::operators::Operator> False::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition
) const {
   return std::make_unique<operators::Empty>();
}

}  // namespace silo::query_engine::filter_expressions