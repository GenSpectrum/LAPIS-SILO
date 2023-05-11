#include "silo/query_engine/operators/empty.h"

#include "roaring/roaring.hh"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

Empty::Empty() = default;

Empty::~Empty() noexcept = default;

std::string Empty::toString(const Database& /*database*/) const {
   return "Empty";
}

Type Empty::type() const {
   return EMPTY;
}

OperatorResult Empty::evaluate() const {
   return OperatorResult{new roaring::Roaring(), nullptr};
}

}  // namespace silo::query_engine::operators
