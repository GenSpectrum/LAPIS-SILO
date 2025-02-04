#include "silo/config/config_repository.h"

#include <filesystem>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "config/config_exception.h"

using silo::config::ConfigException;
using silo::config::ConfigRepository;
using silo::config::DatabaseConfig;
using silo::config::ValueType;

class ConfigReaderMock : public silo::config::DatabaseConfigReader {
  public:
   ConfigReaderMock() = default;
   ConfigReaderMock(const ConfigReaderMock& /*other*/){};

   MOCK_METHOD((DatabaseConfig), readConfig, (const std::filesystem::path&), (const override));
};

namespace {

ConfigReaderMock mockConfigReader(const std::string& config_yaml) {
   const ConfigReaderMock config_reader_mock;

   YAML::Node config = YAML::Load(config_yaml);

   EXPECT_CALL(config_reader_mock, readConfig(testing::_))
      .WillRepeatedly(testing::Return(config.as<silo::config::DatabaseConfig>()));

   return config_reader_mock;
}

}  // namespace

TEST(ConfigRepository, shouldReadConfigWithoutErrors) {
   const auto config_reader_mock = mockConfigReader(
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "testPrimaryKey"
      type: "string"
    - name: "metadata1"
      type: "string"
      generateIndex: true
      generateLineageIndex: true
    - name: "metadata2"
      type: "date"
  primaryKey: "testPrimaryKey"
  partitionBy: "metadata1"
)"
   );

   ASSERT_NO_THROW(ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml"));
}

TEST(ConfigRepository, shouldThrowIfPrimaryKeyIsNotInMetadata) {
   const auto config_reader_mock = mockConfigReader(
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "notPrimaryKey"
      type: "string"
  primaryKey: "testPrimaryKey"
)"
   );

   ASSERT_THROW(
      ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml"), ConfigException
   );
}

TEST(ConfigRepository, shouldThrowIfThereAreTwoMetadataWithTheSameName) {
   const auto config_reader_mock = mockConfigReader(
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "testPrimaryKey"
      type: "string"
    - name: "sameName"
      type: "string"
    - name: "sameName"
      type: "date"
  primaryKey: "testPrimaryKey"
)"
   );

   ASSERT_THROW(
      ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml"), ConfigException
   );
}

TEST(ConfigRepository, givenConfigWithDateToSortByThatIsNotConfiguredThenThrows) {
   const auto config_reader_mock = mockConfigReader(
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "testPrimaryKey"
      type: "string"
  primaryKey: "testPrimaryKey"
  dateToSortBy: "notConfiguredDateToSortBy"
)"
   );

   EXPECT_THAT(
      [&config_reader_mock]() {
         ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml");
      },
      ThrowsMessage<ConfigException>(
         ::testing::HasSubstr("dateToSortBy 'notConfiguredDateToSortBy' is not in metadata")
      )
   );
}

TEST(ConfigRepository, givenDateToSortByThatIsNotADateThenThrows) {
   const auto config_reader_mock = mockConfigReader(
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "testPrimaryKey"
      type: "string"
    - name: "not a date"
      type: "string"
  primaryKey: "testPrimaryKey"
  dateToSortBy: "not a date"
)"
   );

   EXPECT_THAT(
      [&config_reader_mock]() {
         ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml");
      },
      ThrowsMessage<ConfigException>(
         ::testing::HasSubstr("dateToSortBy 'not a date' must be of type DATE")
      )
   );
}

TEST(ConfigRepository, givenConfigPartitionByThatIsNotConfiguredThenThrows) {
   const auto config_reader_mock = mockConfigReader(
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "testPrimaryKey"
      type: "string"
    - name: "dateToSortBy"
      type: "date"
  primaryKey: "testPrimaryKey"
  dateToSortBy: "dateToSortBy"
  partitionBy: "notConfiguredPartitionBy"
)"
   );

   EXPECT_THAT(
      [&config_reader_mock]() {
         ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml");
      },
      ThrowsMessage<ConfigException>(
         ::testing::HasSubstr("partitionBy 'notConfiguredPartitionBy' is not in metadata")
      )
   );
}

TEST(ConfigRepository, givenConfigPartitionByThatIsNotALineageThrows) {
   const auto config_reader_mock = mockConfigReader(
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "testPrimaryKey"
      type: "string"
    - name: "dateToSortBy"
      type: "date"
    - name: "not a lineage"
      type: "string"
  primaryKey: "testPrimaryKey"
  dateToSortBy: "dateToSortBy"
  partitionBy: "not a lineage"
)"
   );
   EXPECT_THAT(
      [&config_reader_mock]() {
         ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml");
      },
      ThrowsMessage<silo::config::ConfigException>(::testing::HasSubstr(
         "partitionBy 'not a lineage' must be of type STRING and needs 'generateLineageIndex' set"
      ))
   );
}

TEST(ConfigRepository, givenMetadataToGenerateIndexForThatIsNotStringThenThrows) {
   const auto config_reader_mock = mockConfigReader(
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "testPrimaryKey"
      type: "string"
    - name: "indexed date"
      type: "date"
      generateIndex: true
  primaryKey: "testPrimaryKey"
  dateToSortBy: null
  partitionBy: "testPrimaryKey"
)"
   );

   EXPECT_THAT(
      [&config_reader_mock]() {
         ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml");
      },
      ThrowsMessage<ConfigException>(
         ::testing::HasSubstr("Metadata 'indexed date' generateIndex is set, but generating an "
                              "index is only allowed for types STRING")
      )
   );
}

TEST(ConfigRepository, givenLineageIndexAndNotGenerateThenThrows) {
   const auto config_reader_mock = mockConfigReader(
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "testPrimaryKey"
      type: "string"
    - name: "some lineage"
      type: "string"
      generateLineageIndex: true
  primaryKey: "testPrimaryKey"
  dateToSortBy: null
  partitionBy: "testPrimaryKey"
)"
   );

   EXPECT_THAT(
      [&config_reader_mock]() {
         ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml");
      },
      ThrowsMessage<ConfigException>(
         ::testing::HasSubstr("Metadata 'some lineage' generateLineageIndex is set, "
                              "generateIndex must also be set")
      )
   );
}
