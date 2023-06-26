#include "silo/common/string.h"
#include "silo/common/bidirectional_map.h"

#include <gtest/gtest.h>

using silo::common::BidirectionalMap;
using silo::common::String;
using silo::common::STRING_SIZE;

TEST(String, correctToString) {
   BidirectionalMap<std::string> dict;
   String<STRING_SIZE> underTest("value 1", dict);

   EXPECT_EQ(underTest.toString(dict), "value 1");
}

TEST(String, correctToStringLong) {
   BidirectionalMap<std::string> dict;
   String<STRING_SIZE> underTest("some longer value 1", dict);

   EXPECT_EQ(underTest.toString(dict), "some longer value 1");
}

TEST(String, correctToStringVeryLong) {
   BidirectionalMap<std::string> dict;
   std::string value =
      "some very long value 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 "
      "6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0";
   String<STRING_SIZE> underTest(value, dict);

   EXPECT_EQ(underTest.toString(dict), value);
}

TEST(String, comparesCorrectlySameValues) {
   BidirectionalMap<std::string> dict;
   std::string value =
      "some very long value 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 "
      "6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0";
   String<STRING_SIZE> underTest1(value, dict);
   String<STRING_SIZE> underTest2(value, dict);

   EXPECT_EQ(underTest1, underTest2);
}

TEST(String, comparesCorrectUnequalValues) {
   BidirectionalMap<std::string> dict;
   std::string value =
      "some very long value 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 "
      "6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0";
   String<STRING_SIZE> underTest1(value, dict);
   String<STRING_SIZE> underTest2(value + " different", dict);

   EXPECT_NE(underTest1, underTest2);
}

TEST(String, comparesCorrectlyIfPrefixesMatchUpTo32Positions) {
   BidirectionalMap<std::string> dict;
   std::string value = "1234567890abcdefghijklmnopqrstuv";
   for (size_t i = 0; i < 32U; ++i) {
      String<STRING_SIZE> underTest1(value.substr(0, i) + "x", dict);
      String<STRING_SIZE> underTest2(value.substr(0, i) + "y", dict);
      EXPECT_NE(underTest1, underTest2);
      String<STRING_SIZE> underTest3(value.substr(0, i) + "y", dict);
      EXPECT_EQ(underTest2, underTest3);
   }
}