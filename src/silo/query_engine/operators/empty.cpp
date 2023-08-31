#include "silo/query_engine/operators/empty.h"

#include <string>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/full.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

Empty::Empty(uint32_t row_count)
    : row_count(row_count){};

Empty::~Empty() noexcept = default;

std::string Empty::toString() const {
   return "Empty";
}

Type Empty::type() const {
   return EMPTY;
}

OperatorResult Empty::evaluate() const {
   return OperatorResult();
}

std::unique_ptr<Operator> Empty::copy() const {
   return std::make_unique<Empty>(row_count);
}

std::unique_ptr<Operator> Empty::negate() const {
   return std::make_unique<Full>(row_count);
}

}  // namespace silo::query_engine::operators
