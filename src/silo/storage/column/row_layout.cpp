#include "silo/storage/column/row_layout.h"

#include <cstdint>

#include <roaring/roaring.hh>

namespace silo::storage::column {

roaring::Roaring RowLayout::fullBitmap() const {
   roaring::Roaring result;
   result.addRange(0, num_rows);
   return result;
}

void RowLayout::complementInPlace(roaring::Roaring& bitmap) const {
   bitmap.flip(0, num_rows);
}

}  // namespace silo::storage::column
