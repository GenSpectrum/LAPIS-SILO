#include "silo/storage/column/int_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

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
