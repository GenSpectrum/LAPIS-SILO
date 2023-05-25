#include "silo/query_engine/operators/bitmap_selection.h"

#include "roaring/roaring.hh"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

BitmapSelection::BitmapSelection(
   const roaring::Roaring* bitmaps,
   unsigned sequence_count,
   Predicate comparator,
   uint32_t value
)
    : bitmaps(bitmaps),
      sequence_count(sequence_count),
      comparator(comparator),
      value(value) {}

BitmapSelection::~BitmapSelection() noexcept = default;

std::string BitmapSelection::toString() const {
   return "BitmapSelection";
}

Type BitmapSelection::type() const {
   return BITMAP_SELECTION;
}

void BitmapSelection::negate() {
   switch (this->comparator) {
      case CONTAINS:
         this->comparator = NOT_CONTAINS;
         break;
      case NOT_CONTAINS:
         this->comparator = CONTAINS;
         break;
   }
}

OperatorResult BitmapSelection::evaluate() const {
   auto* bitmap = new roaring::Roaring();
   switch (this->comparator) {
      case CONTAINS:
         for (unsigned i = 0; i < sequence_count; i++) {
            if (bitmaps[i].contains(value)) {
               bitmap->add(i);
            }
         }
         break;
      case NOT_CONTAINS:
         for (unsigned i = 0; i < sequence_count; i++) {
            if (!bitmaps[i].contains(value)) {
               bitmap->add(i);
            }
         }
         break;
   }
   return OperatorResult(bitmap);
}

}  // namespace silo::query_engine::operators
