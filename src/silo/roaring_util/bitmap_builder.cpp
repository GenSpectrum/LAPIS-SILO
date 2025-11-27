#include "silo/roaring_util/bitmap_builder.h"

namespace silo::roaring_util {

void BitmapBuilderByContainer::addContainer(
   uint16_t v_index,
   roaring::internal::container_t* container,
   uint8_t typecode
) {
   if (current_v_tile_index > v_index) {
      throw std::runtime_error(
         "containers must be inserted into BitmapBuilderByContainer by increasing v_index"
      );
   }
   // first container received
   if (current_container == nullptr) {
      current_v_tile_index = v_index;
      current_typecode = typecode;
      current_container = roaring::internal::container_clone(container, typecode);
   }
   // current_container is finished
   else if (current_v_tile_index != v_index) {
      roaring::internal::ra_append(
         &result_bitmap.roaring.high_low_container,
         current_v_tile_index,
         current_container,
         current_typecode
      );
      current_v_tile_index = v_index;
      current_typecode = typecode;
      current_container = roaring::internal::container_clone(container, typecode);
   }
   // add a container to current_container
   else { /* current_v_tile_index == sequence_diff_key.v_index */
      uint8_t result_typecode;
      roaring::internal::container_t* result_container = roaring::internal::container_ior(
         current_container, current_typecode, container, typecode, &result_typecode
      );
      if (result_container != current_container) {
         roaring::internal::container_free(current_container, current_typecode);
         current_container = result_container;
         current_typecode = result_typecode;
      }
   }
}

roaring::Roaring BitmapBuilderByContainer::getBitmap() && {
   if (current_container != nullptr) {
      roaring::internal::ra_append(
         &result_bitmap.roaring.high_low_container,
         current_v_tile_index,
         current_container,
         current_typecode
      );
   }
   return std::move(result_bitmap);
}

void BitmapBuilderByRange::add(uint32_t pos) {
   if (pos == current_range_end) {
      current_range_end++;
   } else {
      flush();
      current_range_start = pos;
      current_range_end = pos + 1;
   }
}

void BitmapBuilderByRange::flush() {
   if (current_range_start < current_range_end) {
      bitmap.addRange(current_range_start, current_range_end);
   }
}

roaring::Roaring BitmapBuilderByRange::getBitmap() && {
   flush();
   return std::move(bitmap);
}

}  // namespace silo::roaring_util
