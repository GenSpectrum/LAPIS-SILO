#include "silo/storage/column/int_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::storage::column::ColumnMetadata;
using silo::storage::column::IntColumn;
using silo::storage::column::RowId;

TEST(IntColumn, doesNotErrorOnValidInputs) {
   ColumnMetadata column_metadata("int_column1");
   IntColumn column{&column_metadata};
   IntColumn::Builder builder;
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
TEST(IntColumn, updateAssignsScalarValueAcrossChunks) {
   ColumnMetadata column_metadata("int_column1");
   IntColumn column{&column_metadata};

   IntColumn::Builder chunk0;
   chunk0.insert(10);
   chunk0.insertNull();  // (0, 1)
   chunk0.insert(30);
   SILO_ASSERT(column.appendChunk(chunk0.finalize()).has_value());

   IntColumn::Builder chunk1;
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
