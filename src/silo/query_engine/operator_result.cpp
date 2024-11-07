#include "silo/query_engine/operator_result.h"

#include <utility>

#include <roaring/roaring.hh>

namespace silo::query_engine {

OperatorResult::OperatorResult()
    : mutable_bitmap(std::make_shared<roaring::Roaring>()),
      immutable_bitmap(nullptr) {}

OperatorResult::OperatorResult(const roaring::Roaring& bitmap)
    : mutable_bitmap(nullptr),
      immutable_bitmap(&bitmap) {}

OperatorResult::OperatorResult(roaring::Roaring&& bitmap)
    : mutable_bitmap(std::make_shared<roaring::Roaring>(std::move(bitmap))),
      immutable_bitmap(nullptr) {}

const roaring::Roaring& OperatorResult::operator*() const {
   return immutable_bitmap ? *immutable_bitmap : *mutable_bitmap;
}

roaring::Roaring& OperatorResult::operator*() {
   if (!mutable_bitmap) {
      mutable_bitmap = std::make_shared<roaring::Roaring>(*immutable_bitmap);
      immutable_bitmap = nullptr;
   }
   return *mutable_bitmap;
}

std::shared_ptr<roaring::Roaring> OperatorResult::operator->() {
   if (!mutable_bitmap) {
      mutable_bitmap = std::make_shared<roaring::Roaring>(*immutable_bitmap);
      immutable_bitmap = nullptr;
   }
   return mutable_bitmap;
}

const roaring::Roaring* OperatorResult::operator->() const {
   return immutable_bitmap ? immutable_bitmap : mutable_bitmap.get();
}

bool OperatorResult::isMutable() const {
   return mutable_bitmap != nullptr;
}

}  // namespace silo::query_engine
