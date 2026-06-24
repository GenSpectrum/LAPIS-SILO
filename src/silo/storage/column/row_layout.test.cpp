#include "silo/storage/column/row_layout.h"

#include <cstdint>
#include <initializer_list>
#include <stdexcept>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <roaring/roaring.hh>

#include "silo/storage/column/column.h"
#include "silo/storage/column/row_id.h"

using silo::storage::column::COLUMN_CHUNK_SIZE;
using silo::storage::column::RowId;
using silo::storage::column::RowLayout;

TEST(RowLayout, ofBuildsLayoutFromVariadicChunkSizes) {
   const RowLayout layout = RowLayout::of(3, 5, 2);

   ASSERT_EQ(layout.numChunks(), 3);
   ASSERT_EQ(layout.chunkSize(0), 3);
   ASSERT_EQ(layout.chunkSize(1), 5);
   ASSERT_EQ(layout.chunkSize(2), 2);
   ASSERT_EQ(layout.numRows(), 10);
}

TEST(RowLayout, ofIsEquivalentToRepeatedAppendChunk) {
   RowLayout appended;
   appended.appendChunk(4);
   appended.appendChunk(6);
   appended.appendChunk(7);

   const RowLayout factory = RowLayout::of(4, 6, 7);

   ASSERT_EQ(factory.numChunks(), appended.numChunks());
   ASSERT_EQ(factory.numRows(), appended.numRows());
   for (size_t chunk_id = 0; chunk_id < factory.numChunks(); ++chunk_id) {
      ASSERT_EQ(factory.chunkSize(chunk_id), appended.chunkSize(chunk_id));
   }
}

TEST(RowLayout, ofWithNoArgumentsIsEmpty) {
   const RowLayout layout = RowLayout::of();

   ASSERT_EQ(layout.numChunks(), 0);
   ASSERT_EQ(layout.numRows(), 0);
   ASSERT_EQ(layout.begin(), layout.end());
}

TEST(RowLayout, defaultConstructedIsEmpty) {
   const RowLayout layout;

   ASSERT_EQ(layout.numChunks(), 0);
   ASSERT_EQ(layout.numRows(), 0);
}

TEST(RowLayout, appendChunkAccumulatesRows) {
   RowLayout layout;
   ASSERT_EQ(layout.numRows(), 0);
   layout.appendChunk(3);
   ASSERT_EQ(layout.numRows(), 3);
   layout.appendChunk(5);
   ASSERT_EQ(layout.numRows(), 8);
}

TEST(RowLayout, iteratorVisitsEveryRowChunkByChunk) {
   const RowLayout layout = RowLayout::of(2, 3);

   std::vector<RowId> visited;
   for (const RowId row_id : layout) {
      visited.push_back(row_id);
   }

   ASSERT_THAT(
      visited,
      ::testing::ElementsAre(
         RowId{.chunk_id = 0, .row_in_chunk = 0},
         RowId{.chunk_id = 0, .row_in_chunk = 1},
         RowId{.chunk_id = 1, .row_in_chunk = 0},
         RowId{.chunk_id = 1, .row_in_chunk = 1},
         RowId{.chunk_id = 1, .row_in_chunk = 2}
      )
   );
}

TEST(RowLayout, iteratorAdvancesPastFullChunkAtUint16Max) {
   // A chunk of exactly COLUMN_CHUNK_SIZE (2^16) rows is the only way the last row's `row_in_chunk`
   // reaches UINT16_MAX, which exercises the first clause of the if-branch in operator++. The full
   // chunk size must also round-trip through the uint16 `row_in_chunk` arithmetic so that advancing
   // past the last row rolls over to the next chunk instead of wrapping back to row 0.
   const RowLayout layout = RowLayout::of(COLUMN_CHUNK_SIZE, 1);

   ASSERT_EQ(layout.chunkSize(0), COLUMN_CHUNK_SIZE);
   ASSERT_EQ(layout.numRows(), static_cast<uint32_t>(COLUMN_CHUNK_SIZE) + 1);

   std::vector<RowId> visited;
   for (const RowId row_id : layout) {
      visited.push_back(row_id);
   }

   ASSERT_EQ(visited.size(), static_cast<size_t>(COLUMN_CHUNK_SIZE) + 1);
   ASSERT_EQ(visited.front(), (RowId{.chunk_id = 0, .row_in_chunk = 0}));
   // The last row of the full chunk sits at UINT16_MAX...
   ASSERT_EQ(visited[COLUMN_CHUNK_SIZE - 1], (RowId{.chunk_id = 0, .row_in_chunk = UINT16_MAX}));
   // ...and advancing past it rolls over into the next chunk rather than overflowing to row 0.
   ASSERT_EQ(visited.back(), (RowId{.chunk_id = 1, .row_in_chunk = 0}));
}

TEST(RowLayout, appendChunkRejectsEmptyChunk) {
   RowLayout layout;
   ASSERT_THROW(layout.appendChunk(0), std::runtime_error);
}

TEST(RowLayout, ofRejectsEmptyChunk) {
   ASSERT_THROW(RowLayout::of(2, 0, 3), std::runtime_error);
}

namespace {
uint32_t global(uint16_t chunk_id, uint16_t row_in_chunk) {
   return RowId{.chunk_id = chunk_id, .row_in_chunk = row_in_chunk}.toGlobal();
}

roaring::Roaring bitmapOf(std::initializer_list<uint32_t> global_row_ids) {
   roaring::Roaring bitmap;
   for (const uint32_t global_row_id : global_row_ids) {
      bitmap.add(global_row_id);
   }
   return bitmap;
}
}  // namespace

TEST(RowLayout, fullBitmapReflectsSparse2To16AlignedLayout) {
   const RowLayout layout = RowLayout::of(2, 3);

   // Exactly the populated rows of each chunk; the 2^16-aligned gap between chunks stays empty.
   const roaring::Roaring expected =
      bitmapOf({global(0, 0), global(0, 1), global(1, 0), global(1, 1), global(1, 2)});

   ASSERT_EQ(layout.fullBitmap(), expected);
}

TEST(RowLayout, complementInPlaceFlipsOnlyPopulatedRanges) {
   const RowLayout layout = RowLayout::of(2, 3);

   roaring::Roaring bitmap = bitmapOf({global(0, 0), global(1, 1)});

   layout.complementInPlace(bitmap);

   // The complement within the universe of valid row ids; gaps between chunks stay empty.
   const roaring::Roaring expected = bitmapOf({global(0, 1), global(1, 0), global(1, 2)});

   ASSERT_EQ(bitmap, expected);
}

TEST(RowLayout, complementInPlaceOfFullBitmapIsEmptyAndViceVersa) {
   const RowLayout layout = RowLayout::of(2, 3);

   // Complementing the full universe yields the empty set.
   roaring::Roaring full = layout.fullBitmap();
   layout.complementInPlace(full);
   ASSERT_TRUE(full.isEmpty());

   // Complementing the empty set yields the full universe back.
   roaring::Roaring empty;
   layout.complementInPlace(empty);
   ASSERT_EQ(empty, layout.fullBitmap());
}
