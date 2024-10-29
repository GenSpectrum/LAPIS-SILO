#include "silo/storage/column/int_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/preprocessing/preprocessing_exception.h"

using silo::storage::column::IntColumn;

TEST(IntColumn, doesNotErrorOnValidInputs) {
   IntColumn column("int_column1");
   auto& column_partition = column.createPartition();
   column_partition.insert("123");
   column_partition.insertNull();
   ASSERT_EQ(column_partition.getValues().size(), 2);
   ASSERT_EQ(column_partition.getValues().at(0), 123);
   ASSERT_EQ(column_partition.getValues().at(1), IntColumn::null());
}

TEST(IntColumn, errorsOnInvalidInteger) {
   IntColumn column("int_column1");
   auto& column_partition = column.createPartition();
   EXPECT_THAT(
      [&]() { column_partition.insert("not an integer"); },
      ThrowsMessage<silo::preprocessing::PreprocessingException>(
         ::testing::HasSubstr("Wrong format for Integer: 'not an integer' in column 'int_column1'")
      )
   );
}
