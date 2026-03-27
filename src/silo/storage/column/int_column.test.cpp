#include "silo/storage/column/int_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using silo::storage::column::ColumnMetadata;
using silo::storage::column::IntColumn;

TEST(IntColumn, doesNotErrorOnValidInputs) {
   ColumnMetadata column_metadata("int_column1");
   IntColumn column{&column_metadata};
   SILO_ASSERT(column.insert(123).has_value());
   column.insertNull();
   ASSERT_EQ(column.numValues(), 2);
   ASSERT_FALSE(column.isNull(0));
   ASSERT_EQ(column.getValue(0), 123);
   ASSERT_TRUE(column.isNull(1));
}
