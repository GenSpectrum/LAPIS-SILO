#include "silo/query_engine/copy_on_write_bitmap.h"

#include <cstdint>
#include <utility>
#include <vector>

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

uint64_t CopyOnWriteBitmap::cardinality() const {
   return getConstReference().cardinality();
}

bool CopyOnWriteBitmap::isEmpty() const {
   return getConstReference().isEmpty();
}

uint64_t CopyOnWriteBitmap::andCardinality(const CopyOnWriteBitmap& other) const {
   return getConstReference().and_cardinality(other.getConstReference());
}

CopyOnWriteBitmap& CopyOnWriteBitmap::operator&=(const CopyOnWriteBitmap& other) {
   getMutable() &= other.getConstReference();
   return *this;
}

CopyOnWriteBitmap& CopyOnWriteBitmap::operator-=(const CopyOnWriteBitmap& other) {
   getMutable() -= other.getConstReference();
   return *this;
}

CopyOnWriteBitmap& CopyOnWriteBitmap::operator|=(const CopyOnWriteBitmap& other) {
   getMutable() |= other.getConstReference();
   return *this;
}

CopyOnWriteBitmap CopyOnWriteBitmap::operator&(const CopyOnWriteBitmap& other) const {
   return CopyOnWriteBitmap{getConstReference() & other.getConstReference()};
}

CopyOnWriteBitmap CopyOnWriteBitmap::operator-(const CopyOnWriteBitmap& other) const {
   return CopyOnWriteBitmap{getConstReference() - other.getConstReference()};
}

CopyOnWriteBitmap CopyOnWriteBitmap::fastUnion(const std::vector<CopyOnWriteBitmap>& bitmaps) {
   std::vector<const roaring::Roaring*> inputs;
   inputs.reserve(bitmaps.size());
   for (const auto& bitmap : bitmaps) {
      inputs.push_back(&bitmap.getConstReference());
   }
   return CopyOnWriteBitmap{roaring::Roaring::fastunion(inputs.size(), inputs.data())};
}

roaring::Roaring CopyOnWriteBitmap::toRoaring() const {
   return getConstReference();
}

}  // namespace silo::query_engine
