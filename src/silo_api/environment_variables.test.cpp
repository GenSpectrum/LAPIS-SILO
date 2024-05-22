#include "silo_api/environment_variables.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

TEST(EnvironmentVariables, correctPrefixedUppercase) {
   ASSERT_EQ(silo_api::EnvironmentVariables::prefixedUppercase({{""}}), "SILO_");
   ASSERT_EQ(silo_api::EnvironmentVariables::prefixedUppercase({{"A"}}), "SILO__A");
   ASSERT_EQ(silo_api::EnvironmentVariables::prefixedUppercase({{"abc"}}), "SILO_ABC");
   ASSERT_EQ(
      silo_api::EnvironmentVariables::prefixedUppercase({{"someCamelCase"}}), "SILO_SOME_CAMEL_CASE"
   );
   ASSERT_EQ(
      silo_api::EnvironmentVariables::prefixedUppercase({{"BADCamelCase"}}),
      "SILO__B_A_D_CAMEL_CASE"
   );
   ASSERT_EQ(
      silo_api::EnvironmentVariables::prefixedUppercase({{"something_with_underscores"}}),
      "SILO_SOMETHING_WITH_UNDERSCORES"
   );
   ASSERT_EQ(
      silo_api::EnvironmentVariables::prefixedUppercase({{"something_with_underscores"}}),
      "SILO_SOMETHING_WITH_UNDERSCORES"
   );
   ASSERT_EQ(
      silo_api::EnvironmentVariables::prefixedUppercase({{"some", "subsectionedSequence"}}),
      "SILO_SOME_SUBSECTIONED_SEQUENCE"
   );
   ASSERT_EQ(
      silo_api::EnvironmentVariables::prefixedUppercase({{"some", "more", "sections"}}),
      "SILO_SOME_MORE_SECTIONS"
   );
}