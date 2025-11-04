#include "silo/query_engine/copy_on_write_bitmap.h"

#include <utility>

#include <roaring/roaring.hh>

namespace silo::query_engine {

CopyOnWriteBitmap::CopyOnWriteBitmap()
    : mutable_bitmap(std::make_shared<roaring::Roaring>()),
      immutable_bitmap(nullptr) {}

CopyOnWriteBitmap::CopyOnWriteBitmap(const roaring::Roaring* bitmap)
    : mutable_bitmap(nullptr),
      immutable_bitmap(bitmap) {}

CopyOnWriteBitmap::CopyOnWriteBitmap(roaring::Roaring&& bitmap)
    : mutable_bitmap(std::make_shared<roaring::Roaring>(std::move(bitmap))),
      immutable_bitmap(nullptr) {}

const roaring::Roaring& CopyOnWriteBitmap::getConstReference() const {
   return immutable_bitmap ? *immutable_bitmap : *mutable_bitmap;
}

roaring::Roaring& CopyOnWriteBitmap::getMutable() {
   if (!mutable_bitmap) {
      mutable_bitmap = std::make_shared<roaring::Roaring>(*immutable_bitmap);
      immutable_bitmap = nullptr;
   }
   return *mutable_bitmap;
}

bool CopyOnWriteBitmap::isMutable() const {
   return mutable_bitmap != nullptr;
}

}  // namespace silo::query_engine
