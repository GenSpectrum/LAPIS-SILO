#include "silo_api/environment_variables.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

TEST(EnvironmentVariables, correctPrefixedUppercase) {
   ASSERT_EQ(silo_api::EnvironmentVariables::prefixedUppercase("abc"), "SILO_ABC");
   ASSERT_EQ(
      silo_api::EnvironmentVariables::prefixedUppercase("someCamelCase"), "SILO_SOME_CAMEL_CASE"
   );
   ASSERT_EQ(
      silo_api::EnvironmentVariables::prefixedUppercase("BADCamelCase"), "SILO__B_A_D_CAMEL_CASE"
   );
   ASSERT_EQ(
      silo_api::EnvironmentVariables::prefixedUppercase("something_with_underscores"),
      "SILO_SOMETHING_WITH_UNDERSCORES"
   );
}