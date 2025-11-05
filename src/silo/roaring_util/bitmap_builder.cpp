#include "silo/roaring_util/bitmap_builder.h"

namespace silo::roaring_util {

void BitmapBuilderByContainer::addContainer(
   uint16_t v_index,
   roaring::internal::container_t* container,
   uint8_t typecode
) {
   SILO_ASSERT(current_v_tile_index <= v_index);
   if (current_v_tile_index != v_index) {
      if (current_container != nullptr) {
         roaring::internal::ra_append(
            &result_bitmap.roaring.high_low_container,
            current_v_tile_index,
            current_container,
            current_typecode
         );
      }
      current_v_tile_index = v_index;
      current_typecode = typecode;
      current_container = roaring::internal::container_clone(container, typecode);
   } else { /* current_v_tile_index == sequence_diff_key.v_index */
      SILO_ASSERT(current_container != nullptr);
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

}  // namespace silo::roaring_util
