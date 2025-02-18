#include "silo/query_engine/filter/operators/bitmap_selection.h"

#include <string>

#include <fmt/format.h>
#include <roaring/roaring.hh>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

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

BitmapSelection::BitmapSelection(
   std::unique_ptr<silo::query_engine::filter::expressions::Expression>&& logical_equivalent,
   const roaring::Roaring* bitmaps,
   uint32_t row_count,
   Comparator comparator,
   uint32_t value
)
    : logical_equivalent(std::move(logical_equivalent)),
      bitmaps(bitmaps),
      row_count(row_count),
      comparator(comparator),
      value(value) {}

BitmapSelection::~BitmapSelection() noexcept = default;

std::string BitmapSelection::toString() const {
   if (logical_equivalent.has_value()) {
      return fmt::format("BitmapSelection({})", logical_equivalent.value()->toString());
   }
   return "BitmapSelection";
}

Type BitmapSelection::type() const {
   return BITMAP_SELECTION;
}

CopyOnWriteBitmap BitmapSelection::evaluate() const {
   CopyOnWriteBitmap bitmap;
   switch (this->comparator) {
      case CONTAINS:
         for (uint32_t i = 0; i < row_count; i++) {
            if (bitmaps[i].contains(value)) {
               bitmap->add(i);
            }
         }
         break;
      case NOT_CONTAINS:
         for (uint32_t i = 0; i < row_count; i++) {
            if (!bitmaps[i].contains(value)) {
               bitmap->add(i);
            }
         }
         break;
   }
   return bitmap;
}

std::unique_ptr<Operator> BitmapSelection::negate(
   std::unique_ptr<BitmapSelection>&& bitmap_selection
) {
   switch (bitmap_selection->comparator) {
      case CONTAINS:
         bitmap_selection->comparator = NOT_CONTAINS;
         break;
      case NOT_CONTAINS:
         bitmap_selection->comparator = CONTAINS;
         break;
   }
   return bitmap_selection;
}

}  // namespace silo::query_engine::filter::operators
