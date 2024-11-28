#include "config/source/environment_variables.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

using silo::config::ConfigKeyPath;
using silo::config::EnvironmentVariables;

TEST(EnvironmentVariables, correctPrefixedUppercase) {
   ASSERT_EQ(
      EnvironmentVariables::configKeyPathToString(ConfigKeyPath::tryFrom({{"a"}}).value()), "SILO_A"
   );
   ASSERT_EQ(
      EnvironmentVariables::configKeyPathToString(ConfigKeyPath::tryFrom({{"abc"}}).value()),
      "SILO_ABC"
   );
   ASSERT_EQ(
      EnvironmentVariables::configKeyPathToString(ConfigKeyPath::tryFrom({{"some", "snake", "case"}}
      ).value()),
      "SILO_SOME_SNAKE_CASE"
   );
   ASSERT_EQ(
      EnvironmentVariables::configKeyPathToString(
         ConfigKeyPath::tryFrom({{"some"}, {"subsectioned", "sequence"}}).value()
      ),
      "SILO_SOME_SUBSECTIONED_SEQUENCE"
   );
   ASSERT_EQ(
      EnvironmentVariables::configKeyPathToString(
         ConfigKeyPath::tryFrom({{"some"}, {"more"}, {"sections"}}).value()
      ),
      "SILO_SOME_MORE_SECTIONS"
   );
}