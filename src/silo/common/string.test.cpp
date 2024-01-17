#include "silo/common/string.h"
#include "silo/common/bidirectional_map.h"

#include <gtest/gtest.h>

using silo::common::BidirectionalMap;
using silo::common::String;
using silo::common::STRING_SIZE;

TEST(String, correctToString) {
   BidirectionalMap<std::string> dict;
   const String<STRING_SIZE> under_test("value 1", dict);

   EXPECT_EQ(under_test.toString(dict), "value 1");
}

TEST(String, correctWithEmptyString) {
   BidirectionalMap<std::string> dict;
   const String<STRING_SIZE> under_test("", dict);

   EXPECT_EQ(under_test.toString(dict), "");
}

TEST(String, correctToStringLong) {
   BidirectionalMap<std::string> dict;
   const String<STRING_SIZE> under_test("some longer value 1", dict);

   EXPECT_EQ(under_test.toString(dict), "some longer value 1");
}

TEST(String, correctToStringVeryLong) {
   BidirectionalMap<std::string> dict;
   const std::string value =
      "some very long value 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 "
      "6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0";
   const String<STRING_SIZE> under_test(value, dict);

   EXPECT_EQ(under_test.toString(dict), value);
}

TEST(String, comparesCorrectlySameValues) {
   BidirectionalMap<std::string> dict;
   const std::string value =
      "some very long value 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 "
      "6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0";
   const String<STRING_SIZE> under_test1(value, dict);
   const String<STRING_SIZE> under_test2(value, dict);

   EXPECT_EQ(under_test1, under_test2);
}

TEST(String, comparesCorrectUnequalValues) {
   BidirectionalMap<std::string> dict;
   const std::string value =
      "some very long value 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 "
      "6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0";
   const String<STRING_SIZE> under_test1(value, dict);
   const String<STRING_SIZE> under_test2(value + " different", dict);

   EXPECT_NE(under_test1, under_test2);
}

TEST(String, comparesCorrectlyIfPrefixesMatchUpTo32Positions) {
   BidirectionalMap<std::string> dict;
   const std::string value = "1234567890abcdefghijklmnopqrstuv";
   for (size_t i = 0; i < value.size(); ++i) {
      const String<STRING_SIZE> under_test1(value.substr(0, i) + "x", dict);
      const String<STRING_SIZE> under_test2(value.substr(0, i) + "y", dict);
      EXPECT_NE(under_test1, under_test2);
      const String<STRING_SIZE> under_test3(value.substr(0, i) + "y", dict);
      EXPECT_EQ(under_test2, under_test3);
   }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(String, fastComparesCorrectlyIfPrefixesMatchUpTo32Positions) {
   BidirectionalMap<std::string> dict;
   const std::string value = "1234567890abcdefghijklmnopqrstuv";
   for (size_t i = 0; i < 16; ++i) {
      const String<STRING_SIZE> under_test1(value.substr(0, i) + "x", dict);
      const String<STRING_SIZE> under_test2(value.substr(0, i) + "y", dict);
      EXPECT_EQ(under_test1.fastCompare(under_test2), std::strong_ordering::less);
      EXPECT_EQ(under_test2.fastCompare(under_test1), std::strong_ordering::greater);
      const String<STRING_SIZE> under_test3(value.substr(0, i) + "y", dict);
      EXPECT_EQ(under_test2.fastCompare(under_test3), std::strong_ordering::equal);
      EXPECT_EQ(under_test3.fastCompare(under_test2), std::strong_ordering::equal);
   }
   for (size_t i = 16; i < value.size(); ++i) {
      const String<STRING_SIZE> under_test1(value.substr(0, i) + "x", dict);
      const String<STRING_SIZE> under_test2(value.substr(0, i) + "y", dict);
      EXPECT_EQ(under_test1.fastCompare(under_test2), std::nullopt);
      EXPECT_EQ(under_test2.fastCompare(under_test1), std::nullopt);
      const String<STRING_SIZE> under_test3(value.substr(0, i) + "y", dict);
      EXPECT_EQ(under_test2.fastCompare(under_test3), std::strong_ordering::equal);
      EXPECT_EQ(under_test3.fastCompare(under_test2), std::strong_ordering::equal);
   }
}