#pragma once

#include <memory>

#include "external/roaring_include_wrapper.h"

namespace silo::query_engine {

/// The return value of the Operator::evaluate method.
/// May return either a mutable or immutable bitmap.
class CopyOnWriteBitmap {
  private:
   std::shared_ptr<roaring::Roaring> mutable_bitmap;
   const roaring::Roaring* immutable_bitmap;

  public:
   CopyOnWriteBitmap();
   explicit CopyOnWriteBitmap(const roaring::Roaring& bitmap);
   explicit CopyOnWriteBitmap(roaring::Roaring&& bitmap);

   roaring::Roaring& operator*();
   const roaring::Roaring& operator*() const;
   std::shared_ptr<roaring::Roaring> operator->();
   const roaring::Roaring* operator->() const;

   [[nodiscard]] bool isMutable() const;
};

}  // namespace silo::query_engine
