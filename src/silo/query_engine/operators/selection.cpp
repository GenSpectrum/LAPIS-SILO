#include "silo/query_engine/operators/selection.h"

#include <roaring/roaring.hh>
#include <vector>

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

Selection::Selection(
   uint64_t const* column,
   Comparator comparator,
   uint64_t value,
   unsigned sequence_count
)
    : column(column),
      comparator(comparator),
      value(value),
      sequence_count(sequence_count) {}

Selection::~Selection() noexcept = default;

std::string Selection::toString() const {
   return "Select";
}

Type Selection::type() const {
   return SELECTION;
}

void Selection::negate() {
   switch (this->comparator) {
      case EQUALS:
         this->comparator = NOT_EQUALS;
         break;
      case NOT_EQUALS:
         this->comparator = EQUALS;
         break;
      case LESS:
         this->comparator = HIGHER_OR_EQUALS;
         break;
      case HIGHER_OR_EQUALS:
         this->comparator = LESS;
         break;
      case HIGHER:
         this->comparator = LESS_OR_EQUALS;
         break;
      case LESS_OR_EQUALS:
         this->comparator = HIGHER;
         break;
   }
}

OperatorResult Selection::evaluate() const {
   auto* result = new roaring::Roaring();
   switch (this->comparator) {
      case EQUALS:
         for (unsigned i = 0; i < sequence_count; i++) {
            if (column[i] == value) {
               result->add(i);
            }
         }
         break;
      case NOT_EQUALS:
         for (unsigned i = 0; i < sequence_count; i++) {
            if (column[i] != value) {
               result->add(i);
            }
         }
         break;
      case LESS:
         for (unsigned i = 0; i < sequence_count; i++) {
            if (column[i] < value) {
               result->add(i);
            }
         }
         break;
      case HIGHER_OR_EQUALS:
         for (unsigned i = 0; i < sequence_count; i++) {
            if (column[i] >= value) {
               result->add(i);
            }
         }
         break;
      case HIGHER:
         for (unsigned i = 0; i < sequence_count; i++) {
            if (column[i] > value) {
               result->add(i);
            }
         }
         break;
      case LESS_OR_EQUALS:
         for (unsigned i = 0; i < sequence_count; i++) {
            if (column[i] <= value) {
               result->add(i);
            }
         }
         break;
   }
   return OperatorResult(result);
}

}  // namespace silo::query_engine::operators
