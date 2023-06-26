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

TEST(IndexedStringColumnPartition, insertedValuesRequeried) {
   silo::common::BidirectionalMap<std::string> lookup;
   IndexedStringColumnPartition under_test(lookup);

   under_test.insert("value 1");
   under_test.insert("value 2");
   under_test.insert("value 2");
   under_test.insert("value 3");
   under_test.insert("value 1");

   // EXPECT_EQ(under_test.getAsString(0U), "value 1");
   // EXPECT_EQ(under_test.getAsString(1U), "value 2");
   // EXPECT_EQ(under_test.getAsString(2U), "value 2");
   // EXPECT_EQ(under_test.getAsString(3U), "value 3");
   // EXPECT_EQ(under_test.getAsString(4U), "value 1");
}
