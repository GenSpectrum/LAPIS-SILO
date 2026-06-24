#pragma once

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>

#include "silo/common/panic.h"

namespace roaring {
class Roaring;
}

namespace silo::storage::column {

using RowId = uint32_t;

/// The shared row layout of a table
class RowLayout {
   uint32_t num_rows = 0;

  public:
   template <typename... ChunkSizes>
   static RowLayout of(ChunkSizes... chunk_sizes) {
      RowLayout layout;
      (layout.appendChunk(static_cast<uint32_t>(chunk_sizes)), ...);
      return layout;
   }

   void appendChunk(uint16_t chunk_size) { num_rows += chunk_size; }

   /// The total number of rows across in the table
   [[nodiscard]] uint32_t numRows() const { return num_rows; }

   /// A bitmap of every valid global row id
   [[nodiscard]] roaring::Roaring fullBitmap() const;

   /// Replace `bitmap` with its complement within the universe of valid row ids: afterwards it
   /// contains exactly the valid row ids it did not contain before. `bitmap` is expected to be a
   /// subset of the universe.
   void complementInPlace(roaring::Roaring& bitmap) const;

   /// Forward iterator yielding the `RowId` of every row in the table
   class Iterator {
      uint32_t row_id = 0;

     public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = RowId;
      using difference_type = std::ptrdiff_t;
      using pointer = const RowId*;
      using reference = RowId;

      Iterator() = default;

      explicit Iterator(uint32_t row_id)
          : row_id(row_id) {}

      RowId operator*() const { return row_id; }

      Iterator& operator++() {
         ++row_id;
         return *this;
      }

      bool operator==(const Iterator& other) const { return row_id == other.row_id; }

      bool operator!=(const Iterator& other) const { return !(*this == other); }
   };

   // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
   [[nodiscard]] Iterator begin() const { return Iterator{0}; }

   [[nodiscard]] Iterator end() const { return Iterator{num_rows}; }

  private:
   friend class boost::serialization::access;
   template <class Archive>
   void serialize(Archive& archive, const uint32_t /* version */) {
      archive & num_rows;
   }
};

}  // namespace silo::storage::column
