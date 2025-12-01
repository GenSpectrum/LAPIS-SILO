#include "silo/common/german_string.h"

#include <gtest/gtest.h>

using silo::GermanString;
using silo::SiloString;

TEST(String, correctToString) {
   const SiloString under_test("value 1");

   EXPECT_TRUE(under_test.isInPlace());
   EXPECT_EQ(under_test.getShortString(), "value 1");
}

TEST(String, correctWithEmptyString) {
   const SiloString under_test("");

   EXPECT_TRUE(under_test.isInPlace());
   EXPECT_EQ(under_test.getShortString(), "");
}

using silo::storage::vector::VariableDataRegistry;
TEST(String, correctlyReturnsSuffixId) {
   const SiloString under_test(
      100, "prfx", VariableDataRegistry::Identifier{.page_id = 0, .offset = 3}
   );

   EXPECT_EQ(under_test.suffixId().page_id, 0);
   EXPECT_EQ(under_test.suffixId().offset, 3);
}

TEST(String, correctlyReturnsLengthLong) {
   const SiloString under_test(
      100, "prfx", VariableDataRegistry::Identifier{.page_id = 0, .offset = 3}
   );

   EXPECT_EQ(under_test.length(), 100);
}

TEST(String, correctlyReturnsLengthInPlace) {
   const SiloString under_test("in_place");

   EXPECT_EQ(under_test.length(), 8);
}
