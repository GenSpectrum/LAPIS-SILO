#include "silo/common/data_version.h"

#include <gtest/gtest.h>

using silo::DataVersion;

TEST(DataVersion, shouldMineDataVersionFromUnixTime) {
   const auto mined_version = DataVersion::mineDataVersion();
   EXPECT_EQ(mined_version.getTimestamp().value.size(), 10UL);
   EXPECT_EQ(mined_version.getTimestamp().value[0], '1');
}

TEST(DataVersion, shouldConstructFromVersionString) {
   const auto timestamp = DataVersion::Timestamp::fromString("1234567890");
   EXPECT_TRUE(timestamp.has_value());
   if (timestamp.has_value()) {
      EXPECT_EQ(timestamp->value, "1234567890");
   }
}

TEST(DataVersion, shouldRejectFalseVersionFromString) {
   const auto timestamp = DataVersion::Timestamp::fromString("3X123");
   EXPECT_FALSE(timestamp.has_value());
}

TEST(DataVersion, shouldConstructWithDefaultVersion) {
   const auto timestamp = DataVersion::Timestamp::fromString("");
   EXPECT_TRUE(timestamp.has_value());
   if (timestamp.has_value()) {
      EXPECT_EQ(timestamp->value, "");
   }
}