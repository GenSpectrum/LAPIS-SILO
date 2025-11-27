#pragma once

#include <roaring/roaring.hh>

#include "silo/common/panic.h"

namespace silo::roaring_util {

class BitmapBuilderByContainer {
   roaring::Roaring result_bitmap;
   uint16_t current_v_tile_index = 0;
   roaring::internal::container_t* current_container = nullptr;
   uint8_t current_typecode = 0;

  public:
   void addContainer(uint16_t v_index, roaring::internal::container_t* container, uint8_t typecode);

   roaring::Roaring getBitmap() &&;
};

class BitmapBuilderByRange {
   roaring::Roaring bitmap;

   uint32_t current_range_start = 0;
   uint32_t current_range_end = 0;

  public:
   void add(uint32_t pos);

   void flush();

   roaring::Roaring getBitmap() &&;
};

}  // namespace silo::roaring_util
