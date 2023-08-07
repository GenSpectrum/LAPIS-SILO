#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

#include "silo/common/pango_lineage.h"
#include "silo/storage/pango_lineage_alias.h"

namespace {

struct TestParameter {
   std::string input;
   std::string expected_result;
};

class ResolveAliasTestFixture : public ::testing::TestWithParam<TestParameter> {
  protected:
   const silo::PangoLineageAliasLookup alias_map =
      silo::PangoLineageAliasLookup{{{"X", {"A"}}, {"XY", {"A.1"}}}};
};

TEST_P(ResolveAliasTestFixture, shouldReturnExpectedResolvedAlias) {
   const auto test_parameter = GetParam();

   const auto result = alias_map.unaliasPangoLineage({test_parameter.input});

   ASSERT_EQ(result.value, test_parameter.expected_result);
}

// NOLINTNEXTLINE(readability-identifier-length)
INSTANTIATE_TEST_SUITE_P(
   ResolveAliasTest,
   ResolveAliasTestFixture,
   ::testing::Values(
      TestParameter{"", ""},
      TestParameter{"SomeNotListedAlias", "SomeNotListedAlias"},
      TestParameter{"X", "A"},
      TestParameter{"XY", "A.1"},
      TestParameter{"X.1.1", "A.1.1"},
      TestParameter{"XYX.1.1", "XYX.1.1"},
      TestParameter{".X", ".X"}
   )
);

TEST(PangoLineageAliasLookup, readFromFile) {
   auto under_test = silo::PangoLineageAliasLookup::readFromFile(
      "testBaseData/exampleDataset/pangolineage_alias.json"
   );

   ASSERT_EQ(under_test.unaliasPangoLineage({"B"}).value, "B");
   ASSERT_EQ(under_test.unaliasPangoLineage({"B.1"}).value, "B.1");
   ASSERT_EQ(under_test.unaliasPangoLineage({"B.1.2"}).value, "B.1.2");
   ASSERT_EQ(under_test.unaliasPangoLineage({"C"}).value, "B.1.1.1");
   ASSERT_EQ(under_test.unaliasPangoLineage({"EP"}).value, "B.1.1.529.2.75.3.1.1.4");
}

TEST(PangoLineageAliasLookup, readFromFileShouldThrowIfFileDoesNotExist) {
   ASSERT_THROW(
      silo::PangoLineageAliasLookup::readFromFile("testBaseData/does_not_exist.json"),
      std::filesystem::filesystem_error
   );
}

TEST(PangoLineageAliasLookup, readFromFileShouldThrowIfFileIsNotJson) {
   ASSERT_THROW(
      silo::PangoLineageAliasLookup::readFromFile("testBaseData/pango_alias.txt"),
      std::filesystem::filesystem_error
   );
}

}  // namespace
