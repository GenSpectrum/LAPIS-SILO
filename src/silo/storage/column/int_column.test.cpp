#include "silo/storage/column/int_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using silo::storage::column::ColumnMetadata;
using silo::storage::column::IntColumnPartition;

TEST(IntColumn, doesNotErrorOnValidInputs) {
   ColumnMetadata column("int_column1");
   IntColumnPartition column_partition{&column};
   column_partition.insert(123);
   column_partition.insertNull();
   ASSERT_EQ(column_partition.numValues(), 2);
   ASSERT_FALSE(column_partition.isNull(0));
   ASSERT_EQ(column_partition.getValue(0), 123);
   ASSERT_TRUE(column_partition.isNull(1));
}
