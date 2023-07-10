#include "silo/config/config_repository.h"

#include <filesystem>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/config/config_exception.h"
#include "silo/config/database_config_reader.h"

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

ConfigReaderMock mockConfigReader(const DatabaseConfig& config) {
   const ConfigReaderMock config_reader_mock;

   EXPECT_CALL(config_reader_mock, readConfig(testing::_)).WillRepeatedly(testing::Return(config));

   return config_reader_mock;
}

TEST(ConfigRepository, shouldReadConfigWithoutErrors) {
   const auto config_reader_mock = mockConfigReader(
      {"main",
       {
          "testInstanceName",
          {
             {"testPrimaryKey", ValueType::STRING},
             {"metadata1", ValueType::PANGOLINEAGE, true},
             {"metadata2", ValueType::DATE},
          },
          "testPrimaryKey",
          std::nullopt,
          "metadata1",
       }}
   );

   ASSERT_NO_THROW(ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml"));
}

TEST(ConfigRepository, shouldThrowIfPrimaryKeyIsNotInMetadata) {
   const auto config_reader_mock = mockConfigReader(
      {"main",
       {
          "testInstanceName",
          {
             {"notPrimaryKey", ValueType::STRING},
          },
          "testPrimaryKey",
       }}
   );

   ASSERT_THROW(
      ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml"), ConfigException
   );
}

TEST(ConfigRepository, shouldThrowIfThereAreTwoMetadataWithTheSameName) {
   const auto config_reader_mock = mockConfigReader(
      {"main",
       {
          "testInstanceName",
          {
             {"testPrimaryKey", ValueType::STRING},
             {"sameName", ValueType::PANGOLINEAGE},
             {"sameName", ValueType::DATE},
          },
          "testPrimaryKey",
       }}
   );

   ASSERT_THROW(
      ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml"), ConfigException
   );
}

TEST(ConfigRepository, givenConfigWithDateToSortByThatIsNotConfiguredThenThrows) {
   const auto config_reader_mock = mockConfigReader(
      {"main",
       {"testInstanceName",
        {
           {"testPrimaryKey", ValueType::STRING},
        },
        "testPrimaryKey",
        "notConfiguredDateToSortBy"}}
   );

   EXPECT_THAT(
      [&config_reader_mock]() {
         ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml");
      },
      ThrowsMessage<ConfigException>(
         ::testing::HasSubstr("date_to_sort_by 'notConfiguredDateToSortBy' is not in metadata")
      )
   );
}

TEST(ConfigRepository, givenDateToSortByThatIsNotADateThenThrows) {
   const auto config_reader_mock = mockConfigReader(
      {"main",
       {"testInstanceName",
        {
           {"testPrimaryKey", ValueType::STRING},
           {"not a date", ValueType::STRING},
        },
        "testPrimaryKey",
        "not a date"}}
   );

   EXPECT_THAT(
      [&config_reader_mock]() {
         ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml");
      },
      ThrowsMessage<ConfigException>(
         ::testing::HasSubstr("date_to_sort_by 'not a date' must be of type DATE")
      )
   );
}

TEST(ConfigRepository, givenConfigPartitionByThatIsNotConfiguredThenThrows) {
   const auto config_reader_mock = mockConfigReader(
      {"main",
       {"testInstanceName",
        {
           {"testPrimaryKey", ValueType::STRING},
           {"date_to_sort_by", ValueType::DATE},
        },
        "testPrimaryKey",
        "date_to_sort_by",
        "notConfiguredPartitionBy"}}
   );

   EXPECT_THAT(
      [&config_reader_mock]() {
         ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml");
      },
      ThrowsMessage<ConfigException>(
         ::testing::HasSubstr("partition_by 'notConfiguredPartitionBy' is not in metadata")
      )
   );
}

TEST(ConfigRepository, givenConfigPartitionByThatIsNotPangoLineageThenThrows) {
   const auto config_reader_mock = mockConfigReader(
      {"main",
       {"testInstanceName",
        {
           {"testPrimaryKey", ValueType::STRING},
           {"date_to_sort_by", ValueType::DATE},
           {"not a pango lineage", ValueType::STRING},
        },
        "testPrimaryKey",
        "date_to_sort_by",
        "not a pango lineage"}}
   );

   EXPECT_THAT(
      [&config_reader_mock]() {
         ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml");
      },
      ThrowsMessage<ConfigException>(
         ::testing::HasSubstr("partition_by 'not a pango lineage' must be of type PANGOLINEAGE")
      )
   );
}

TEST(ConfigRepository, givenMetadataToGenerateIndexForThatIsNotStringOrPangoLineageThenThrows) {
   const auto config_reader_mock = mockConfigReader(
      {"main",
       {"testInstanceName",
        {
           {"testPrimaryKey", ValueType::STRING},
           {"indexed date", ValueType::DATE, true},
        },
        "testPrimaryKey",
        std::nullopt,
        "testPrimaryKey"}}
   );

   EXPECT_THAT(
      [&config_reader_mock]() {
         ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml");
      },
      ThrowsMessage<ConfigException>(
         ::testing::HasSubstr("Metadata 'indexed date' generate_index is set, but generating an "
                              "index is only allowed for types STRING and PANGOLINEAGE")
      )
   );
}

TEST(ConfigRepository, givenNotGenerateIndexOnPangoLineageThenThrows) {
   const auto config_reader_mock = mockConfigReader(
      {"main",
       {"testInstanceName",
        {
           {"testPrimaryKey", ValueType::STRING},
           {"pango lineage without index", ValueType::PANGOLINEAGE, false},
        },
        "testPrimaryKey",
        std::nullopt,
        "testPrimaryKey"}}
   );

   EXPECT_THAT(
      [&config_reader_mock]() {
         ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml");
      },
      ThrowsMessage<ConfigException>(
         ::testing::HasSubstr("Metadata 'pango lineage without index' generate_index is not set, "
                              "but generating an index is mandatory for type PANGOLINEAGE")
      )
   );
}
