#include "silo/common/base64.h"

#include <string>
#include <string_view>

#include <gtest/gtest.h>
#include <simdutf.h>

TEST(Base64, givenInvalidCharacterReturnsSensibleError) {
   // '!' is not a valid base64 character
   // simdutf reports INVALID_BASE64_CHARACTER which we turn into an error message
   const std::string_view invalid = "!!!!";

   const auto result = silo::decodeBase64(invalid);
   ASSERT_FALSE(result.has_value());
   EXPECT_EQ(result.error(), "the encoded string contained an invalid base64 character");
}

TEST(Base64, givenInvalidLengthReturnsSensibleError) {
   // the length must be a multiple of two
   const std::string_view invalid = "aaaAA";

   const auto result = silo::decodeBase64(invalid);
   ASSERT_FALSE(result.has_value());
   EXPECT_EQ(result.error(), "invalid padding of base64 input");
}
