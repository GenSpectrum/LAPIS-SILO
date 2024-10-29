#include "silo/storage/column/string_column.h"

#include <gtest/gtest.h>

using silo::storage::column::StringColumn;
using silo::storage::column::StringColumnPartition;

TEST(StringColumnPartition, rawInsertedValuesRequeried) {
   silo::common::BidirectionalMap<std::string> lookup;
   StringColumnPartition under_test("string_column", &lookup);

   under_test.insert("value 1");
   under_test.insert("value 2");
   under_test.insert("value 2");
   under_test.insert("value 3");
   under_test.insert("some string that is a little longer 1");
   under_test.insert("value 1");

   EXPECT_EQ(under_test.getValues()[0].toString(lookup), "value 1");
   EXPECT_EQ(under_test.getValues()[1].toString(lookup), "value 2");
   EXPECT_EQ(under_test.getValues()[2].toString(lookup), "value 2");
   EXPECT_EQ(under_test.getValues()[3].toString(lookup), "value 3");
   EXPECT_EQ(under_test.getValues()[4].toString(lookup), "some string that is a little longer 1");
   EXPECT_EQ(under_test.getValues()[5].toString(lookup), "value 1");
}

TEST(StringColumn, rawInsertedValuesRequeried) {
   StringColumn column("string_column");
   StringColumnPartition under_test = column.createPartition();

   under_test.insert("value 1");
   under_test.insert("value 2");
   under_test.insert("value 2");
   under_test.insert("value 3");
   under_test.insert("some string that is a little longer 1");
   under_test.insert("value 1");

   const silo::common::String somehow_acquired_element_representation = under_test.getValues()[4];

   EXPECT_EQ(
      under_test.lookupValue(somehow_acquired_element_representation),
      "some string that is a little longer 1"
   );
}

TEST(StringColumn, compareAcrossPartitions) {
   StringColumn under_test("string_column");
   StringColumnPartition partition_1 = under_test.createPartition();
   partition_1.insert("value 1");
   partition_1.insert("value 2");
   partition_1.insert("value 2");
   partition_1.insert("value 3");
   partition_1.insert("some string that is a little longer 1");
   partition_1.insert("value 1");

   StringColumnPartition partition_2 = under_test.createPartition();
   partition_2.insert("other value 2");
   partition_2.insert("other values 3");
   partition_2.insert("value 1");
   partition_2.insert("other value 3");
   partition_2.insert("some string that is a little longer 1");
   partition_2.insert("other value 1");

   EXPECT_EQ(partition_1.getValues()[0], partition_1.getValues()[5]);
   EXPECT_EQ(partition_1.getValues()[5], partition_2.getValues()[2]);
   EXPECT_EQ(partition_1.getValues()[4], partition_2.getValues()[4]);
}
