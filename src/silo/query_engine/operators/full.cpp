#include "silo/query_engine/operators/full.h"

#include <roaring/roaring.hh>

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

Full::Full(uint32_t sequenceCount)
    : sequenceCount(sequenceCount) {}

Full::~Full() noexcept = default;

std::string Full::toString(const Database& /*database*/) const {
   return "Full";
}

Type Full::type() const {
   return FULL;
}

OperatorResult Full::evaluate() const {
   auto* result = new roaring::Roaring();
   result->addRange(0, sequenceCount);
   return {result, nullptr};
}

}  // namespace silo::query_engine::operators
