#include "silo/query_engine/operators/bitmap_selection.h"

#include "roaring/roaring.hh"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

BitmapSelection::BitmapSelection(
   const roaring::Roaring* bitmaps,
   uint32_t row_count,
   Comparator comparator,
   uint32_t value
)
    : bitmaps(bitmaps),
      row_count(row_count),
      comparator(comparator),
      value(value) {}

BitmapSelection::~BitmapSelection() noexcept = default;

std::string BitmapSelection::toString() const {
   return "BitmapSelection";
}

Type BitmapSelection::type() const {
   return BITMAP_SELECTION;
}

OperatorResult BitmapSelection::evaluate() const {
   auto* bitmap = new roaring::Roaring();
   switch (this->comparator) {
      case CONTAINS:
         for (unsigned i = 0; i < row_count; i++) {
            if (bitmaps[i].contains(value)) {
               bitmap->add(i);
            }
         }
         break;
      case NOT_CONTAINS:
         for (unsigned i = 0; i < row_count; i++) {
            if (!bitmaps[i].contains(value)) {
               bitmap->add(i);
            }
         }
         break;
   }
   return OperatorResult(bitmap);
}

std::unique_ptr<Operator> BitmapSelection::copy() const {
   return std::make_unique<BitmapSelection>(bitmaps, row_count, comparator, value);
}

std::unique_ptr<Operator> BitmapSelection::negate() const {
   auto ret = std::make_unique<BitmapSelection>(*this);
   switch (this->comparator) {
      case CONTAINS:
         ret->comparator = NOT_CONTAINS;
         break;
      case NOT_CONTAINS:
         ret->comparator = CONTAINS;
         break;
   }
   return ret;
}

}  // namespace silo::query_engine::operators
