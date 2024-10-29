#include "silo/storage/column/float_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/preprocessing/preprocessing_exception.h"

using silo::storage::column::FloatColumn;

TEST(FloatColumn, doesNotErrorOnValidInputs) {
   FloatColumn column("float_column1");
   auto& column_partition = column.createPartition();
   column_partition.insert("0.1");
   column_partition.insertNull();
   ASSERT_EQ(column_partition.getValues().size(), 2);
   ASSERT_EQ(column_partition.getValues().at(0), 0.1);
   ASSERT_TRUE(std::isnan(column_partition.getValues().at(1)));
}

TEST(FloatColumn, errorsOnInvalidFloat) {
   FloatColumn column("float_column1");
   auto& column_partition = column.createPartition();
   EXPECT_THAT(
      [&]() { column_partition.insert("not a float"); },
      ThrowsMessage<silo::preprocessing::PreprocessingException>(::testing::HasSubstr(
         "Bad format for double value: 'not a float' in column 'float_column1'"
      ))
   );
}
