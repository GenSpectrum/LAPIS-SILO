#include "silo/query_engine/operator_result.h"

#include <utility>

#include <roaring/roaring.hh>

namespace silo::query_engine {

OperatorResult::OperatorResult()
    : mutable_bitmap(new roaring::Roaring()),
      immutable_bitmap(nullptr) {}

OperatorResult::OperatorResult(const roaring::Roaring& bitmap)
    : mutable_bitmap(nullptr),
      immutable_bitmap(&bitmap) {}

OperatorResult::OperatorResult(roaring::Roaring&& bitmap)
    : mutable_bitmap(new roaring::Roaring(std::move(bitmap))),
      immutable_bitmap(nullptr) {}

OperatorResult::~OperatorResult() {
   delete mutable_bitmap;
}

OperatorResult::OperatorResult(OperatorResult&& other) noexcept  // move constructor
    : mutable_bitmap(std::exchange(other.mutable_bitmap, nullptr)),
      immutable_bitmap(other.immutable_bitmap) {}

OperatorResult& OperatorResult::operator=(OperatorResult&& other) noexcept  // move assignment
{
   std::swap(mutable_bitmap, other.mutable_bitmap);
   std::swap(immutable_bitmap, other.immutable_bitmap);
   return *this;
}

const roaring::Roaring& OperatorResult::operator*() const {
   return mutable_bitmap ? *mutable_bitmap : *immutable_bitmap;
}

roaring::Roaring& OperatorResult::operator*() {
   if (!mutable_bitmap) {
      mutable_bitmap = new roaring::Roaring(*immutable_bitmap);
      immutable_bitmap = nullptr;
   }
   return *mutable_bitmap;
}

roaring::Roaring* OperatorResult::operator->() {
   if (!mutable_bitmap) {
      mutable_bitmap = new roaring::Roaring(*immutable_bitmap);
      immutable_bitmap = nullptr;
   }
   return mutable_bitmap;
}

const roaring::Roaring* OperatorResult::operator->() const {
   return mutable_bitmap ? mutable_bitmap : immutable_bitmap;
}

bool OperatorResult::isMutable() const {
   return mutable_bitmap != nullptr;
}

}  // namespace silo::query_engine
