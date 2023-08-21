#include "silo/common/data_version.h"

#include <gtest/gtest.h>

using silo::DataVersion;

TEST(DataVersion, shouldMineDataVersionFromUnixTime) {
   const auto mined_version = DataVersion::mineDataVersion();
   EXPECT_EQ(mined_version.toString().size(), 10);
   EXPECT_EQ(mined_version.toString()[0], '1');
}

TEST(DataVersion, shouldConstructFromVersionString) {
   const auto version = DataVersion::fromString("1234567890");
   EXPECT_TRUE(version.has_value());
   if (version.has_value()) {
      EXPECT_EQ(version->toString(), "1234567890");
   }
}

TEST(DataVersion, shouldRejectFalseVersionFromString) {
   const auto version = DataVersion::fromString("3X123");
   EXPECT_FALSE(version.has_value());
}

TEST(DataVersion, shouldConstructWithDefaultVersion) {
   const auto version = DataVersion::fromString("");
   EXPECT_TRUE(version.has_value());
   if (version.has_value()) {
      EXPECT_EQ(version->toString(), "");
   }
}