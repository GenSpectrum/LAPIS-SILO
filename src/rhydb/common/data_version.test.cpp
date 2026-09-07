#include "rhydb/common/data_version.h"

#include <gtest/gtest.h>

using rhydb::DataVersion;

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

// Data versions have a resolution of one second, but each update has to produce a version of its
// own: it names the directory the state is saved under, and readers use it to tell data apart.
TEST(DataVersion, shouldMineAVersionThatIsNewerThanThePreviousOne) {
   auto previous = DataVersion::mineDataVersion();
   for (size_t update = 0; update < 5; ++update) {
      const auto next = DataVersion::mineDataVersionAfter(previous);
      EXPECT_GT(next, previous);
      EXPECT_EQ(next.getTimestamp().value.size(), 10UL);
      previous = next;
   }
}
