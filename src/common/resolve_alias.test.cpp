#include "gtest/gtest.h"
#include <silo/common/silo_symbols.h>

struct TestParameter {
   std::string input;
   std::string expected_result;
};

class ResolveAliasTestFixture : public ::testing::TestWithParam<TestParameter> {
   protected:
   const std::unordered_map<std::string, std::string> alias_map =
      {
         {"X", "A"},
         {"XY", "A.1"}};
};

TEST_P(ResolveAliasTestFixture, should_return_expected_resolved_alias) {
   const auto test_parameter = GetParam();

   const auto result = silo::resolve_alias(alias_map, test_parameter.input);

   ASSERT_EQ(result, test_parameter.expected_result);
}

INSTANTIATE_TEST_SUITE_P(
   resolve_alias_test,
   ResolveAliasTestFixture,
   ::testing::Values(
      TestParameter{"", ""},
      TestParameter{"SomeNotListedAlias", "SomeNotListedAlias"},
      TestParameter{"X", "A"},
      TestParameter{"XY", "A.1"},
      TestParameter{"X.1.1", "A.1.1"},
      TestParameter{"XYX.1.1", "XYX.1.1"},
      TestParameter{".X", ".X"}));
