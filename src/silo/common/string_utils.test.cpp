#include "silo/common/string_utils.h"

#include <gtest/gtest.h>

using silo::splitBy;

TEST(splitBy, correctSplit) {
   const std::string input("ABC,DEF,ADS,");
   const std::string delimiter(",");

   const auto result = splitBy(input, delimiter);
   EXPECT_EQ(result, std::vector<std::string>({"ABC", "DEF", "ADS", ""}));
}

TEST(splitBy, correctSplit2) {
   const std::string input("ABC");
   const std::string delimiter(",");

   const auto result = splitBy(input, delimiter);
   EXPECT_EQ(result, std::vector<std::string>({"ABC"}));
}

TEST(splitBy, correctSplit3) {
   const std::string input("ABC.*");
   const std::string delimiter(".*");

   const auto result = splitBy(input, delimiter);
   EXPECT_EQ(result, std::vector<std::string>({"ABC", ""}));
}

TEST(splitBy, correctWithEmptyString) {
   const std::string input;
   const std::string_view delimiter(",");

   const auto result = splitBy(input, delimiter);
   EXPECT_EQ(result, std::vector<std::string>{""});
}