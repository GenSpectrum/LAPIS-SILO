#pragma once

#include <roaring/roaring.hh>

#include "silo/common/panic.h"

namespace silo::roaring_util {

class BitmapBuilderByContainer {
   roaring::Roaring result_bitmap;
   int32_t current_v_tile_index = -1;
   roaring::internal::container_t* current_container = nullptr;
   uint8_t current_typecode = 0;

  public:
   void addContainer(uint16_t v_index, roaring::internal::container_t* container, uint8_t typecode);

   roaring::Roaring getBitmap() &&;
};

}  // namespace silo::roaring_util
