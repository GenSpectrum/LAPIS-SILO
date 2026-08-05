#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <roaring/roaring.hh>

namespace silo::query_engine {

/// The return value of the Operator::evaluate method.
/// May return either a mutable or immutable bitmap.
///
/// Set-algebra (AND, OR, ANDNOT and the matching cardinalities) is exposed as member operators so
/// callers never have to reach for the underlying `roaring::Roaring` -- `toRoaring` materializes a
/// standalone copy only at the very end of a query, where the result leaves the engine. The
/// operators are currently thin wrappers over `roaring::Roaring`; keeping every caller on this API
/// lets the internal representation change later without touching call sites.
class CopyOnWriteBitmap {
   std::shared_ptr<roaring::Roaring> mutable_bitmap;
   const roaring::Roaring* immutable_bitmap;

   [[nodiscard]] roaring::Roaring& getMutable();

   [[nodiscard]] const roaring::Roaring& getConstReference() const;

   [[nodiscard]] bool isMutable() const;

  public:
   CopyOnWriteBitmap();

   // A CopyOnWriteBitmap pointing into an immutable bitmap should only be constructed
   // when the CopyOnWriteBitmap's lifetime is contained by bitmap
   explicit CopyOnWriteBitmap(const roaring::Roaring* bitmap);

   explicit CopyOnWriteBitmap(roaring::Roaring&& bitmap);

   [[nodiscard]] uint64_t cardinality() const;

   [[nodiscard]] bool isEmpty() const;

   /// Iterates the contained row ids in ascending order, enabling range-based for over the bitmap.
   /// The iterators borrow the underlying bitmap, so it must outlive them and must not be mutated
   /// while an iteration is in progress.
   [[nodiscard]] roaring::Roaring::const_iterator begin() const;
   [[nodiscard]] roaring::Roaring::const_iterator end() const;

   /// Cardinality of the intersection with `other`, without materializing it.
   [[nodiscard]] uint64_t andCardinality(const CopyOnWriteBitmap& other) const;

   CopyOnWriteBitmap& operator&=(const CopyOnWriteBitmap& other);
   CopyOnWriteBitmap& operator-=(const CopyOnWriteBitmap& other);
   CopyOnWriteBitmap& operator|=(const CopyOnWriteBitmap& other);

   [[nodiscard]] CopyOnWriteBitmap operator&(const CopyOnWriteBitmap& other) const;
   [[nodiscard]] CopyOnWriteBitmap operator-(const CopyOnWriteBitmap& other) const;

   /// Union of many bitmaps.
   [[nodiscard]] static CopyOnWriteBitmap fastUnion(const std::vector<CopyOnWriteBitmap>& bitmaps);

   /// Materializes into a standalone `roaring::Roaring`. Intended for the end of a query only,
   /// where the result is handed to a consumer -- not for intermediate computation.
   [[nodiscard]] roaring::Roaring toRoaring() const;
};

}  // namespace silo::query_engine
