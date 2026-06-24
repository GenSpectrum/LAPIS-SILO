#include "silo/storage/column/row_layout.h"

#include <cstdint>

#include <roaring/roaring.hh>

namespace silo::storage::column {

roaring::Roaring RowLayout::fullBitmap() const {
   roaring::Roaring result;
   for (size_t chunk_id = 0; chunk_id < chunk_sizes.size(); ++chunk_id) {
      const uint64_t start = RowId::chunkStart(static_cast<uint16_t>(chunk_id));
      result.addRange(start, start + chunk_sizes[chunk_id]);
   }
   return result;
}

void RowLayout::complementInPlace(roaring::Roaring& bitmap) const {
   for (size_t chunk_id = 0; chunk_id < chunk_sizes.size(); ++chunk_id) {
      const uint64_t start = RowId::chunkStart(static_cast<uint16_t>(chunk_id));
      bitmap.flip(start, start + chunk_sizes[chunk_id]);
   }
}

}  // namespace silo::storage::column
