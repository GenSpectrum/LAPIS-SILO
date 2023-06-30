#include "silo/storage/column/indexed_string_column.h"

#include <gtest/gtest.h>

using silo::storage::column::IndexedStringColumnPartition;

TEST(IndexedStringColumn, shouldReturnTheCorrectFilteredValues) {
   silo::common::BidirectionalMap<std::string> lookup;
   IndexedStringColumnPartition under_test(lookup);

   under_test.insert("value 1");
   under_test.insert("value 2");
   under_test.insert("value 2");
   under_test.insert("value 3");
   under_test.insert("value 1");

   const auto result1 = under_test.filter("value 1");
   ASSERT_EQ(result1, roaring::Roaring({0, 4}));

   const auto result2 = under_test.filter("value 2");
   ASSERT_EQ(result2, roaring::Roaring({1, 2}));

   const auto result3 = under_test.filter("value that does not exist");
   ASSERT_EQ(result3, roaring::Roaring());
}

TEST(IndexedStringColumnPartition, insertValuesToPartition) {
   silo::common::BidirectionalMap<std::string> lookup;
   IndexedStringColumnPartition under_test(lookup);

   under_test.insert("value 1");
   under_test.insert("value 2");
   under_test.insert("value 2");
   under_test.insert("value 3");
   under_test.insert("value 1");

   EXPECT_EQ(under_test.getValues()[0], 0U);
   EXPECT_EQ(under_test.getValues()[1], 1U);
   EXPECT_EQ(under_test.getValues()[2], 1U);
   EXPECT_EQ(under_test.getValues()[3], 2U);
   EXPECT_EQ(under_test.getValues()[4], 0U);

   EXPECT_EQ(under_test.lookupValue(0U), "value 1");
   EXPECT_EQ(under_test.lookupValue(1U), "value 2");
   EXPECT_EQ(under_test.lookupValue(2U), "value 3");
}
