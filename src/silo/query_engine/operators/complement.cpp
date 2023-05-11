#include "silo/query_engine/operators/complement.h"

#include "roaring/roaring.hh"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

Complement::Complement(std::unique_ptr<Operator> child, uint32_t sequence_count)
    : child(std::move(child)),
      sequence_count(sequence_count) {}

Complement::~Complement() noexcept = default;

std::string Complement::toString(const Database& database) const {
   return "!" + child->toString(database);
}

Type Complement::type() const {
   return COMPLEMENT;
}

OperatorResult Complement::evaluate() const {
   auto tmp = child->evaluate();
   auto* result = tmp.mutable_res ? tmp.mutable_res : new roaring::Roaring(*tmp.immutable_res);
   result->flip(0, sequence_count);
   return {result, nullptr};
}

}  // namespace silo::query_engine::operators
