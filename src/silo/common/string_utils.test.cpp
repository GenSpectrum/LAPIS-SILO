#include "silo/common/string_utils.h"

#include <gtest/gtest.h>

using silo::splitBy;
using std::string;
using std::string_view;
using std::vector;

TEST(splitBy, correctSplit) {
   const string input("ABC,DEF,ADS,");
   const string delimiter(",");

   const auto result = splitBy(input, delimiter);
   EXPECT_EQ(result, vector<string>({"ABC", "DEF", "ADS", ""}));
}

TEST(splitBy, correctSplit2) {
   const string input("ABC");
   const string delimiter(",");

   const auto result = splitBy(input, delimiter);
   EXPECT_EQ(result, vector<string>({"ABC"}));
}

TEST(splitBy, correctSplit3) {
   const string input("ABC.*");
   const string delimiter(".*");

   const auto result = splitBy(input, delimiter);
   EXPECT_EQ(result, vector<string>({"ABC", ""}));
}

TEST(splitBy, correctWithEmptyString) {
   const string input;
   const string_view delimiter(",");

   const auto result = splitBy(input, delimiter);
   EXPECT_EQ(result, vector<string>{""});
}

TEST(removeSymbol, removesAllOccurences) {
   const string input(R"(ABC"DEF"ADS")");

   const auto result = silo::removeSymbol(input, '\"');
   EXPECT_EQ(result, string("ABCDEFADS"));
}

TEST(removeSymbol, removesAtBeginning) {
   const string input(R"("ABC)");

   const auto result = silo::removeSymbol(input, '\"');
   EXPECT_EQ(result, string("ABC"));
}

TEST(removeSymbol, removesAtEnd) {
   const string input(R"(ABC")");

   const auto result = silo::removeSymbol(input, '\"');
   EXPECT_EQ(result, string("ABC"));
}

TEST(removeSymbol, removesAtBeginningAndEnd) {
   const string input(R"("ABC")");

   const auto result = silo::removeSymbol(input, '\"');
   EXPECT_EQ(result, string("ABC"));
}

TEST(removeSymbol, doesNotRemoveIfNotContained) {
   const string input("ABCDEFADS");

   const auto result = silo::removeSymbol(input, '\"');
   EXPECT_EQ(result, string("ABCDEFADS"));
}
