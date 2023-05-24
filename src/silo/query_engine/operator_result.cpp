#include "silo/query_engine/operator_result.h"

namespace silo::query_engine {

OperatorResult::OperatorResult()
    : mutable_bitmap(new roaring::Roaring()),
      immutable_bitmap(nullptr) {}

OperatorResult::OperatorResult(const roaring::Roaring* bitmap)
    : mutable_bitmap(nullptr),
      immutable_bitmap(bitmap) {}

OperatorResult::OperatorResult(roaring::Roaring* bitmap)
    : mutable_bitmap(bitmap),
      immutable_bitmap(nullptr) {}

const roaring::Roaring* OperatorResult::getConst() const {
   return mutable_bitmap ? mutable_bitmap : immutable_bitmap;
}

roaring::Roaring* OperatorResult::getMutable() {
   if (!mutable_bitmap) {
      mutable_bitmap = new roaring::Roaring(*immutable_bitmap);
      immutable_bitmap = nullptr;
   }
   return mutable_bitmap;
}

bool OperatorResult::isMutable() const {
   return mutable_bitmap != nullptr;
}

void OperatorResult::free() const {
   delete mutable_bitmap;
}

}  // namespace silo::query_engine
