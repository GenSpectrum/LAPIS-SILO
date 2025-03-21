#include "silo/storage/column/int_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/preprocessing/preprocessing_exception.h"

using silo::storage::column::ColumnMetadata;
using silo::storage::column::IntColumnPartition;

TEST(IntColumn, doesNotErrorOnValidInputs) {
   ColumnMetadata column("int_column1");
   IntColumnPartition column_partition{&column};
   column_partition.insert(123);
   column_partition.insertNull();
   ASSERT_EQ(column_partition.getValues().size(), 2);
   ASSERT_EQ(column_partition.getValues().at(0), 123);
   ASSERT_EQ(column_partition.getValues().at(1), IntColumnPartition::null());
}
