#include "silo/storage/column/float_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/preprocessing/preprocessing_exception.h"

using silo::storage::column::ColumnMetadata;
using silo::storage::column::FloatColumnPartition;

TEST(FloatColumn, doesNotErrorOnValidInputs) {
   ColumnMetadata column("float_column1");
   FloatColumnPartition column_partition{&column};
   column_partition.insert(0.1);
   column_partition.insertNull();
   ASSERT_EQ(column_partition.getValues().size(), 2);
   ASSERT_EQ(column_partition.getValues().at(0), 0.1);
   ASSERT_TRUE(std::isnan(column_partition.getValues().at(1)));
}
