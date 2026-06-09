#include "silo/storage/column/float_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using silo::storage::column::ColumnMetadata;
using silo::storage::column::FloatColumn;

TEST(FloatColumn, doesNotErrorOnValidInputs) {
   ColumnMetadata column_metadata("float_column1");
   FloatColumn column{&column_metadata};
   FloatColumn::Builder builder;
   builder.insert(0.1);
   builder.insertNull();
   SILO_ASSERT(column.appendChunk(builder.finalize()).has_value());
   ASSERT_EQ(column.numValues(), 2);
   ASSERT_FALSE(column.isNull(0));
   ASSERT_EQ(column.getValue(0), 0.1);
   ASSERT_TRUE(column.isNull(1));
}
