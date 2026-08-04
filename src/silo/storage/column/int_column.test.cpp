#include "silo/storage/column/int_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::storage::column::ColumnMetadata;
using silo::storage::column::Int32Column;
using silo::storage::column::Int64Column;
using silo::storage::column::RowId;

TEST(Int32Column, doesNotErrorOnValidInputs) {
   ColumnMetadata column_metadata("int_column1");
   Int32Column column{&column_metadata};
   Int32Column::Builder builder;
   builder.insert(123);
   builder.insertNull();
   SILO_ASSERT(column.appendChunk(builder.finalize()).has_value());
   ASSERT_EQ(column.numChunks(), 1);
   ASSERT_EQ(column.chunkSize(0), 2);
   ASSERT_FALSE(column.isNull(RowId(0, 0)));
   ASSERT_EQ(column.getValue(RowId(0, 0)), 123);
   ASSERT_TRUE(column.isNull(RowId(0, 1)));
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(Int32Column, updateAssignsScalarValueAcrossChunks) {
   ColumnMetadata column_metadata("int_column1");
   Int32Column column{&column_metadata};

   Int32Column::Builder chunk0;
   chunk0.insert(10);
   chunk0.insertNull();  // (0, 1)
   chunk0.insert(30);
   SILO_ASSERT(column.appendChunk(chunk0.finalize()).has_value());

   Int32Column::Builder chunk1;
   chunk1.insert(40);
   chunk1.insert(50);
   SILO_ASSERT(column.appendChunk(chunk1.finalize()).has_value());

   ASSERT_EQ(column.numChunks(), 2);

   // Assign a single scalar to a set of rows spanning both column chunks, including the previously
   // null row (0, 1), which becomes non-null.
   roaring::Roaring row_ids;
   row_ids.add(RowId(0, 1).toGlobal());
   row_ids.add(RowId(0, 2).toGlobal());
   row_ids.add(RowId(1, 0).toGlobal());
   column.update(row_ids, 77);

   ASSERT_FALSE(column.isNull(RowId(0, 0)));
   ASSERT_EQ(column.getValue(RowId(0, 0)), 10);  // untouched
   ASSERT_FALSE(column.isNull(RowId(0, 1)));
   ASSERT_EQ(column.getValue(RowId(0, 1)), 77);  // null -> 77
   ASSERT_FALSE(column.isNull(RowId(0, 2)));
   ASSERT_EQ(column.getValue(RowId(0, 2)), 77);
   ASSERT_FALSE(column.isNull(RowId(1, 0)));
   ASSERT_EQ(column.getValue(RowId(1, 0)), 77);
   ASSERT_FALSE(column.isNull(RowId(1, 1)));
   ASSERT_EQ(column.getValue(RowId(1, 1)), 50);  // untouched

   // A nullopt update clears the rows.
   roaring::Roaring to_clear;
   to_clear.add(RowId(0, 2).toGlobal());
   column.update(to_clear, std::nullopt);
   ASSERT_TRUE(column.isNull(RowId(0, 2)));
   ASSERT_FALSE(column.isNull(RowId(1, 0)));
   ASSERT_EQ(column.getValue(RowId(1, 0)), 77);
}

TEST(Int64Column, doesNotErrorOnValidInputs) {
   ColumnMetadata column_metadata("int64_column1");
   Int64Column column{&column_metadata};
   Int64Column::Builder builder;
   builder.insert(123);
   builder.insertNull();
   SILO_ASSERT(column.appendChunk(builder.finalize()).has_value());
   ASSERT_EQ(column.numChunks(), 1);
   ASSERT_EQ(column.chunkSize(0), 2);
   ASSERT_FALSE(column.isNull(RowId(0, 0)));
   ASSERT_EQ(column.getValue(RowId(0, 0)), 123);
   ASSERT_TRUE(column.isNull(RowId(0, 1)));
}

TEST(Int64Column, storesValuesBeyondInt32Range) {
   ColumnMetadata column_metadata("int64_column1");
   Int64Column column{&column_metadata};
   Int64Column::Builder builder;
   const int64_t large_positive = 5'000'000'000LL;   // > INT32_MAX
   const int64_t large_negative = -5'000'000'000LL;  // < INT32_MIN
   builder.insert(large_positive);
   builder.insert(large_negative);
   builder.insertNull();
   SILO_ASSERT(column.appendChunk(builder.finalize()).has_value());
   ASSERT_EQ(column.getValue(RowId(0, 0)), large_positive);
   ASSERT_EQ(column.getValue(RowId(0, 1)), large_negative);
   ASSERT_TRUE(column.isNull(RowId(0, 2)));
}

// TODO reuse tests above? make generic? Execute twice for 32 and 64?
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(Int64Column, updateAssignsScalarValueAcrossChunks) {
   ColumnMetadata column_metadata("int64_column1");
   Int64Column column{&column_metadata};

   Int64Column::Builder chunk0;
   chunk0.insert(10);
   chunk0.insertNull();  // (0, 1)
   chunk0.insert(30);
   SILO_ASSERT(column.appendChunk(chunk0.finalize()).has_value());

   Int64Column::Builder chunk1;
   chunk1.insert(40);
   chunk1.insert(50);
   SILO_ASSERT(column.appendChunk(chunk1.finalize()).has_value());

   ASSERT_EQ(column.numChunks(), 2);

   // Assign a single scalar to a set of rows spanning both column chunks, including the previously
   // null row (0, 1), which becomes non-null.
   const int64_t large_value = 9'000'000'000LL;  // > INT32_MAX
   roaring::Roaring row_ids;
   row_ids.add(RowId(0, 1).toGlobal());
   row_ids.add(RowId(0, 2).toGlobal());
   row_ids.add(RowId(1, 0).toGlobal());
   column.update(row_ids, large_value);

   ASSERT_FALSE(column.isNull(RowId(0, 0)));
   ASSERT_EQ(column.getValue(RowId(0, 0)), 10);  // untouched
   ASSERT_FALSE(column.isNull(RowId(0, 1)));
   ASSERT_EQ(column.getValue(RowId(0, 1)), large_value);  // null -> value
   ASSERT_FALSE(column.isNull(RowId(0, 2)));
   ASSERT_EQ(column.getValue(RowId(0, 2)), large_value);
   ASSERT_FALSE(column.isNull(RowId(1, 0)));
   ASSERT_EQ(column.getValue(RowId(1, 0)), large_value);
   ASSERT_FALSE(column.isNull(RowId(1, 1)));
   ASSERT_EQ(column.getValue(RowId(1, 1)), 50);  // untouched

   // A nullopt update clears the rows.
   roaring::Roaring to_clear;
   to_clear.add(RowId(0, 2).toGlobal());
   column.update(to_clear, std::nullopt);
   ASSERT_TRUE(column.isNull(RowId(0, 2)));
   ASSERT_FALSE(column.isNull(RowId(1, 0)));
   ASSERT_EQ(column.getValue(RowId(1, 0)), large_value);
}
