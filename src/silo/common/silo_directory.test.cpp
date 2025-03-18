#include "silo/common/silo_directory.h"

#include <gtest/gtest.h>

using silo::SiloDirectory;

TEST(DatabaseDirectoryWatcher, validBackwardsCompatible) {
   auto under_test =
      silo::SiloDataSource::checkValidDataSource("testBaseData/dataDirectories/1234");
   ASSERT_FALSE(under_test.data_version.isCompatibleVersion());
   ASSERT_EQ(
      under_test.data_version.getTimestamp(), silo::DataVersion::Timestamp::fromString("1234")
   );
}

TEST(DatabaseDirectoryWatcher, validNewFormatOldVersion) {
   auto under_test =
      silo::SiloDataSource::checkValidDataSource("testBaseData/dataDirectories/1235");
   ASSERT_FALSE(under_test.data_version.isCompatibleVersion());
   ASSERT_EQ(
      under_test.data_version.getTimestamp(), silo::DataVersion::Timestamp::fromString("1235")
   );
}

TEST(DatabaseDirectoryWatcher, validNewFormatCurrentVersion) {
   auto under_test =
      silo::SiloDataSource::checkValidDataSource("testBaseData/dataDirectories/1237");
   ASSERT_TRUE(under_test.data_version.isCompatibleVersion());
   ASSERT_EQ(
      under_test.data_version.getTimestamp(), silo::DataVersion::Timestamp::fromString("1237")
   );
}

TEST(DatabaseDirectoryWatcher, validNewFormatIncompatible) {
   auto under_test =
      silo::SiloDataSource::checkValidDataSource("testBaseData/dataDirectories/9999999999991234");
   ASSERT_FALSE(under_test.data_version.isCompatibleVersion());
   ASSERT_EQ(
      under_test.data_version.getTimestamp(),
      silo::DataVersion::Timestamp::fromString("9999999999991234")
   );
}

TEST(DatabaseDirectoryWatcher, invalidFormat) {
   ASSERT_ANY_THROW(silo::SiloDataSource::checkValidDataSource("testBaseData/dataDirectories/3123")
   );
}

TEST(DatabaseDirectoryWatcher, invalidUInt32) {
   ASSERT_ANY_THROW(silo::SiloDataSource::checkValidDataSource("testBaseData/dataDirectories/3124")
   );
}

TEST(DatabaseDirectoryWatcher, invalidYAML) {
   ASSERT_ANY_THROW(silo::SiloDataSource::checkValidDataSource("testBaseData/dataDirectories/3125")
   );
}

TEST(DatabaseDirectoryWatcher, getsMostRecentCompatible) {
   auto under_test = SiloDirectory{"testBaseData/dataDirectories"};
   auto most_recent = under_test.getMostRecentDataDirectory();
   ASSERT_TRUE(most_recent.has_value());
   ASSERT_EQ(most_recent.value().path, "testBaseData/dataDirectories/1237");
   ASSERT_TRUE(most_recent.value().data_version.isCompatibleVersion());
   ASSERT_EQ(
      most_recent.value().data_version.getTimestamp(),
      silo::DataVersion::Timestamp::fromString("1237")
   );
}
