#include "silo/storage/column/string_column.h"

#include <gtest/gtest.h>

using silo::storage::column::IndexedStringColumn;
using silo::storage::column::RawStringColumn;

TEST(IndexedStringColumn, shouldReturnTheCorrectFilteredValues) {
   IndexedStringColumn under_test;

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

TEST(IndexedStringColumn, insertedValuesRequeried) {
   IndexedStringColumn under_test;

   under_test.insert("value 1");
   under_test.insert("value 2");
   under_test.insert("value 2");
   under_test.insert("value 3");
   under_test.insert("value 1");

   EXPECT_EQ(under_test.getAsString(0u), "value 1");
   EXPECT_EQ(under_test.getAsString(1u), "value 2");
   EXPECT_EQ(under_test.getAsString(2u), "value 2");
   EXPECT_EQ(under_test.getAsString(3u), "value 3");
   EXPECT_EQ(under_test.getAsString(4u), "value 1");
}

TEST(RawStringColumn, rawInsertedValuesRequeried) {
   RawStringColumn under_test;

   under_test.insert("value 1");
   under_test.insert("value 2");
   under_test.insert("value 2");
   under_test.insert("value 3");
   under_test.insert("value 1");

   EXPECT_EQ(under_test.getAsString(0u), "value 1");
   EXPECT_EQ(under_test.getAsString(1u), "value 2");
   EXPECT_EQ(under_test.getAsString(2u), "value 2");
   EXPECT_EQ(under_test.getAsString(3u), "value 3");
   EXPECT_EQ(under_test.getAsString(4u), "value 1");
}
