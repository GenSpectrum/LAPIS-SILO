#include "silo/common/numeric_conversion.h"

#include <gtest/gtest.h>

using silo::NumericConversionException;
using silo::tryConvertStringToU32;

TEST(tryConvertStringToU32, correctConversion) {
   const std::string input("12345");
   const uint32_t expected = 12345;

   const auto result = tryConvertStringToU32(input);
   EXPECT_EQ(result, expected);
}

TEST(tryConvertStringToU32, throwOnInvalidArgument) {
   const std::string input("++++");

   EXPECT_THROW(tryConvertStringToU32(input), NumericConversionException);
}

TEST(tryConvertStringToU32, throwOnOutOfRange) {
   const std::string input = std::to_string(1ULL << 63);

   EXPECT_THROW(tryConvertStringToU32(input), NumericConversionException);
}