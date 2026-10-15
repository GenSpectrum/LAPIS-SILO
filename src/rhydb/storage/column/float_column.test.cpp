#include "rhydb/storage/column/float_column.h"

#include <optional>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <roaring/roaring.hh>

#include "rhydb/common/bitmap.h"

using rhydb::storage::column::ColumnMetadata;
using rhydb::storage::column::FloatColumn;
using rhydb::storage::column::RowId;

TEST(FloatColumn, doesNotErrorOnValidInputs) {
   ColumnMetadata column_metadata("float_column1");
   FloatColumn column{&column_metadata};
   FloatColumn::Builder builder;
   builder.insert(0.1);
   builder.insertNull();
   RHYDB_ASSERT(column.appendChunk(builder.finalize()).has_value());
   ASSERT_EQ(column.numChunks(), 1);
   ASSERT_EQ(column.chunkSize(0), 2);
   ASSERT_FALSE(column.isNull(RowId(0, 0)));
   ASSERT_EQ(column.getValue(RowId(0, 0)), 0.1);
   ASSERT_TRUE(column.isNull(RowId(0, 1)));
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(FloatColumn, updateChunkAssignsEachRowItsOwnValue) {
   ColumnMetadata column_metadata("float_column1");
   FloatColumn column{&column_metadata};
   RHYDB_ASSERT(column.appendChunk({0.1, std::nullopt, 0.3}).has_value());
   RHYDB_ASSERT(column.appendChunk({0.4, 0.5, std::nullopt}).has_value());

   // In the second chunk: 0.4 -> null, null -> 7.5; the row between them is untouched.
   const roaring::Roaring rows{RowId(1, 0).toGlobal(), RowId(1, 2).toGlobal()};
   for (const auto& [chunk_id, chunk_rows] : rhydb::Bitmap{&rows}) {
      column.update(chunk_id, chunk_rows, {std::nullopt, 7.5});
   }

   EXPECT_TRUE(column.isNull(RowId(1, 0)));
   EXPECT_EQ(column.getValue(RowId(1, 1)), 0.5);
   EXPECT_FALSE(column.isNull(RowId(1, 2)));
   EXPECT_EQ(column.getValue(RowId(1, 2)), 7.5);
   // The first chunk is untouched.
   EXPECT_EQ(column.getValue(RowId(0, 0)), 0.1);
   EXPECT_TRUE(column.isNull(RowId(0, 1)));
   EXPECT_EQ(column.getValue(RowId(0, 2)), 0.3);
}
