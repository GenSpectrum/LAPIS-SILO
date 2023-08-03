#include "silo/common/data_version.h"

#include <gtest/gtest.h>

using silo::DataVersion;

TEST(DataVersion, shouldMineDataVersionFromUnixTime) {
   const auto mined_version = DataVersion::mineDataVersion();
   EXPECT_EQ(mined_version.size(), 10);
   EXPECT_EQ(mined_version[0], '1');
}

TEST(DataVersion, shouldConstructFromVersionString) {
   const auto version = DataVersion("1234567890");
   EXPECT_EQ(version.toString(), "1234567890");
}

TEST(DataVersion, shouldConstructWithDefaultVersion) {
   const auto version = DataVersion();
   EXPECT_EQ(version.toString(), "");
}