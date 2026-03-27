#include "silo/storage/column/float_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using silo::storage::column::ColumnMetadata;
using silo::storage::column::FloatColumn;

TEST(FloatColumn, doesNotErrorOnValidInputs) {
   ColumnMetadata column_metadata("float_column1");
   FloatColumn column{&column_metadata};
   SILO_ASSERT(column.insert(0.1).has_value());
   column.insertNull();
   ASSERT_EQ(column.numValues(), 2);
   ASSERT_EQ(column.getValue(0), 0.1);
   ASSERT_TRUE(std::isnan(column.getValue(1)));
}
