#include "silo_api/database_directory_watcher.h"

#include <gtest/gtest.h>

using silo_api::DatabaseDirectoryWatcher;

TEST(DatabaseDirectoryWatcher, emptyDirectoryNotValid) {
   ASSERT_EQ(DatabaseDirectoryWatcher::checkValidDataSource(""), std::nullopt);
}

TEST(DatabaseDirectoryWatcher, invalidDirectoryNotValid) {
   ASSERT_EQ(DatabaseDirectoryWatcher::checkValidDataSource("///"), std::nullopt);
}

TEST(DatabaseDirectoryWatcher, validBackwardsCompatible) {
   auto under_test =
      DatabaseDirectoryWatcher::checkValidDataSource("testBaseData/dataDirectories/1234");
   ASSERT_TRUE(under_test.has_value());
   ASSERT_FALSE(under_test.value().isCompatibleVersion());
   ASSERT_EQ(under_test->getTimestamp(), silo::DataVersion::Timestamp::fromString("1234"));
}

TEST(DatabaseDirectoryWatcher, validNewFormatOldVersion) {
   auto under_test =
      DatabaseDirectoryWatcher::checkValidDataSource("testBaseData/dataDirectories/1235");
   ASSERT_TRUE(under_test.has_value());
   ASSERT_FALSE(under_test.value().isCompatibleVersion());
   ASSERT_EQ(under_test->getTimestamp(), silo::DataVersion::Timestamp::fromString("1235"));
}

TEST(DatabaseDirectoryWatcher, validNewFormatCurrentVersion) {
   auto under_test =
      DatabaseDirectoryWatcher::checkValidDataSource("testBaseData/dataDirectories/1237");
   ASSERT_TRUE(under_test.has_value());
   ASSERT_TRUE(under_test.value().isCompatibleVersion());
   ASSERT_EQ(under_test->getTimestamp(), silo::DataVersion::Timestamp::fromString("1237"));
}

TEST(DatabaseDirectoryWatcher, validNewFormatIncompatible) {
   auto under_test =
      DatabaseDirectoryWatcher::checkValidDataSource("testBaseData/dataDirectories/9999999999991234"
      );
   ASSERT_TRUE(under_test.has_value());
   ASSERT_FALSE(under_test.value().isCompatibleVersion());
   ASSERT_EQ(
      under_test->getTimestamp(), silo::DataVersion::Timestamp::fromString("9999999999991234")
   );
}

TEST(DatabaseDirectoryWatcher, invalidFormat) {
   auto under_test =
      DatabaseDirectoryWatcher::checkValidDataSource("testBaseData/dataDirectories/3123");
   ASSERT_FALSE(under_test.has_value());
}

TEST(DatabaseDirectoryWatcher, invalidUInt32) {
   auto under_test =
      DatabaseDirectoryWatcher::checkValidDataSource("testBaseData/dataDirectories/3124");
   ASSERT_FALSE(under_test.has_value());
}

TEST(DatabaseDirectoryWatcher, invalidYAML) {
   auto under_test =
      DatabaseDirectoryWatcher::checkValidDataSource("testBaseData/dataDirectories/3125");
   ASSERT_FALSE(under_test.has_value());
}

TEST(DatabaseDirectoryWatcher, getsMostRecentCompatible) {
   auto under_test =
      DatabaseDirectoryWatcher::getMostRecentDataDirectory("testBaseData/dataDirectories");
   ASSERT_TRUE(under_test.has_value());
   ASSERT_EQ(under_test.value().first, "testBaseData/dataDirectories/1237");
   ASSERT_TRUE(under_test.value().second.isCompatibleVersion());
   ASSERT_EQ(
      under_test.value().second.getTimestamp(), silo::DataVersion::Timestamp::fromString("1237")
   );
}
