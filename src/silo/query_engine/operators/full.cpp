#include "silo/query_engine/operators/full.h"

#include <roaring/roaring.hh>

#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

Full::Full(uint32_t row_count)
    : row_count(row_count) {}

Full::~Full() noexcept = default;

std::string Full::toString() const {
   return "Full";
}

Type Full::type() const {
   return FULL;
}

OperatorResult Full::evaluate() const {
   auto* result = new roaring::Roaring();
   result->addRange(0, row_count);
   return OperatorResult(result);
}

std::unique_ptr<Operator> Full::copy() const {
   return std::make_unique<Full>(row_count);
}

std::unique_ptr<Operator> Full::negate() const {
   return std::make_unique<Empty>(row_count);
}

}  // namespace silo::query_engine::operators
