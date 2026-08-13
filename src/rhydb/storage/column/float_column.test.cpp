#include "rhydb/storage/column/float_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using rhydb::storage::column::ColumnMetadata;
using rhydb::storage::column::FloatColumn;
using rhydb::storage::column::RowId;

TEST(FloatColumn, doesNotErrorOnValidInputs) {
   ColumnMetadata column_metadata("float_column1");
   FloatColumn column{&column_metadata};
   FloatColumn::Builder builder;
   builder.insert(0.1);
   builder.insertNull();
   SILO_ASSERT(column.appendChunk(builder.finalize()).has_value());
   ASSERT_EQ(column.numChunks(), 1);
   ASSERT_EQ(column.chunkSize(0), 2);
   ASSERT_FALSE(column.isNull(RowId(0, 0)));
   ASSERT_EQ(column.getValue(RowId(0, 0)), 0.1);
   ASSERT_TRUE(column.isNull(RowId(0, 1)));
}
