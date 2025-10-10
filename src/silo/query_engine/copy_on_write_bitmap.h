#pragma once

#include <memory>

#include <roaring/roaring.hh>

namespace silo::query_engine {

/// The return value of the Operator::evaluate method.
/// May return either a mutable or immutable bitmap.
class CopyOnWriteBitmap {
  private:
   std::shared_ptr<roaring::Roaring> mutable_bitmap;
   const roaring::Roaring* immutable_bitmap;

  public:
   CopyOnWriteBitmap();

   // A CopyOnWriteBitmap pointing into an immutable bitmap should only be constructed
   // when the CopyOnWriteBitmap's lifetime is contained by bitmap
   explicit CopyOnWriteBitmap(const roaring::Roaring* bitmap);

   explicit CopyOnWriteBitmap(roaring::Roaring&& bitmap);

   roaring::Roaring& operator*();
   const roaring::Roaring& operator*() const;
   std::shared_ptr<roaring::Roaring> operator->();
   const roaring::Roaring* operator->() const;

   [[nodiscard]] bool isMutable() const;
};

}  // namespace silo::query_engine
