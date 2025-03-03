#include "silo/common/silo_directory.h"

#include <gtest/gtest.h>

using silo::SiloDirectory;

TEST(DatabaseDirectoryWatcher, validBackwardsCompatible) {
   auto data_version = SiloDirectory::checkValidDataSource("testBaseData/dataDirectories/1234");
   ASSERT_TRUE(data_version.has_value());
   ASSERT_FALSE(data_version.value().isCompatibleVersion());
   ASSERT_EQ(data_version->getTimestamp(), silo::DataVersion::Timestamp::fromString("1234"));
}

TEST(DatabaseDirectoryWatcher, validNewFormatOldVersion) {
   auto data_version = SiloDirectory::checkValidDataSource("testBaseData/dataDirectories/1235");
   ASSERT_TRUE(data_version.has_value());
   ASSERT_FALSE(data_version.value().isCompatibleVersion());
   ASSERT_EQ(data_version->getTimestamp(), silo::DataVersion::Timestamp::fromString("1235"));
}

TEST(DatabaseDirectoryWatcher, validNewFormatCurrentVersion) {
   auto data_version = SiloDirectory::checkValidDataSource("testBaseData/dataDirectories/1237");
   ASSERT_TRUE(data_version.has_value());
   ASSERT_TRUE(data_version.value().isCompatibleVersion());
   ASSERT_EQ(data_version->getTimestamp(), silo::DataVersion::Timestamp::fromString("1237"));
}

TEST(DatabaseDirectoryWatcher, validNewFormatIncompatible) {
   auto data_version = SiloDirectory::checkValidDataSource("testBaseData/dataDirectories/9999999999991234"
      );
   ASSERT_TRUE(data_version.has_value());
   ASSERT_FALSE(data_version.value().isCompatibleVersion());
   ASSERT_EQ(
      data_version->getTimestamp(), silo::DataVersion::Timestamp::fromString("9999999999991234")
   );
}

TEST(DatabaseDirectoryWatcher, invalidFormat) {
   auto data_version = SiloDirectory::checkValidDataSource("testBaseData/dataDirectories/3123");
   ASSERT_FALSE(data_version.has_value());
}

TEST(DatabaseDirectoryWatcher, invalidUInt32) {
   auto data_version = SiloDirectory::checkValidDataSource("testBaseData/dataDirectories/3124");
   ASSERT_FALSE(data_version.has_value());
}

TEST(DatabaseDirectoryWatcher, invalidYAML) {
   auto data_version = SiloDirectory::checkValidDataSource("testBaseData/dataDirectories/3125");
   ASSERT_FALSE(data_version.has_value());
}

TEST(DatabaseDirectoryWatcher, getsMostRecentCompatible) {
   auto under_test = SiloDirectory{"testBaseData/dataDirectories"};
   auto most_recent = under_test.getMostRecentDataDirectory();
   ASSERT_TRUE(most_recent.has_value());
   ASSERT_EQ(most_recent.value().first, "testBaseData/dataDirectories/1237");
   ASSERT_TRUE(most_recent.value().second.isCompatibleVersion());
   ASSERT_EQ(
      most_recent.value().second.getTimestamp(), silo::DataVersion::Timestamp::fromString("1237")
   );
}
