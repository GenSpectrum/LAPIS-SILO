#include "silo/query_engine/filter/expressions/false.h"

#include <memory>
#include <string>

#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::expressions {

False::False() = default;

std::string False::toString() const {
   return "False";
}

std::unique_ptr<Expression> False::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<False>();
}

std::unique_ptr<operators::Operator> False::compile(const storage::Table& table) const {
   return std::make_unique<operators::Empty>(table.sequence_count);
}

}  // namespace silo::query_engine::filter::expressions