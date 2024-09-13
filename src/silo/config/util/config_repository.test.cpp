#include "silo/config/util/config_repository.h"

#include <filesystem>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/config/util/config_exception.h"

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

ConfigReaderMock mockConfigReader(const DatabaseConfig& config) {
   const ConfigReaderMock config_reader_mock;

   EXPECT_CALL(config_reader_mock, readConfig(testing::_)).WillRepeatedly(testing::Return(config));

   return config_reader_mock;
}

}  // namespace

TEST(ConfigRepository, shouldReadConfigWithoutErrors) {
   const auto config_reader_mock = mockConfigReader(
      {.default_nucleotide_sequence = "main",
       .schema =
          {
             .instance_name = "testInstanceName",
             .metadata =
                {
                   {.name = "testPrimaryKey", .type = ValueType::STRING},
                   {.name = "metadata1", .type = ValueType::STRING, .generate_index = true},
                   {.name = "metadata2", .type = ValueType::DATE},
                },
             .primary_key = "testPrimaryKey",
             .date_to_sort_by = std::nullopt,
             .partition_by = "metadata1",
          }}
   );

   ASSERT_NO_THROW(ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml"));
}

TEST(ConfigRepository, shouldThrowIfPrimaryKeyIsNotInMetadata) {
   const auto config_reader_mock = mockConfigReader(
      {.default_nucleotide_sequence = "main",
       .schema =
          {
             .instance_name = "testInstanceName",
             .metadata =
                {
                   {.name = "notPrimaryKey", .type = ValueType::STRING},
                },
             .primary_key = "testPrimaryKey",
          }}
   );

   ASSERT_THROW(
      ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml"), ConfigException
   );
}

TEST(ConfigRepository, shouldThrowIfThereAreTwoMetadataWithTheSameName) {
   const auto config_reader_mock = mockConfigReader(
      {.default_nucleotide_sequence = "main",
       .schema =
          {
             .instance_name = "testInstanceName",
             .metadata =
                {
                   {.name = "testPrimaryKey", .type = ValueType::STRING},
                   {.name = "sameName", .type = ValueType::STRING},
                   {.name = "sameName", .type = ValueType::DATE},
                },
             .primary_key = "testPrimaryKey",
          }}
   );

   ASSERT_THROW(
      ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml"), ConfigException
   );
}

TEST(ConfigRepository, givenConfigWithDateToSortByThatIsNotConfiguredThenThrows) {
   const auto config_reader_mock = mockConfigReader(
      {.default_nucleotide_sequence = "main",
       .schema =
          {.instance_name = "testInstanceName",
           .metadata =
              {
                 {.name = "testPrimaryKey", .type = ValueType::STRING},
              },
           .primary_key = "testPrimaryKey",
           .date_to_sort_by = "notConfiguredDateToSortBy"}}
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
      {.default_nucleotide_sequence = "main",
       .schema =
          {.instance_name = "testInstanceName",
           .metadata =
              {
                 {.name = "testPrimaryKey", .type = ValueType::STRING},
                 {.name = "not a date", .type = ValueType::STRING},
              },
           .primary_key = "testPrimaryKey",
           .date_to_sort_by = "not a date"}}
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
      {.default_nucleotide_sequence = "main",
       .schema =
          {.instance_name = "testInstanceName",
           .metadata =
              {
                 {.name = "testPrimaryKey", .type = ValueType::STRING},
                 {.name = "date_to_sort_by", .type = ValueType::DATE},
              },
           .primary_key = "testPrimaryKey",
           .date_to_sort_by = "date_to_sort_by",
           .partition_by = "notConfiguredPartitionBy"}}
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

TEST(ConfigRepository, givenConfigPartitionByThatIsNotPangoLineageDoesNotThrow) {
   const auto config_reader_mock = mockConfigReader(
      {.default_nucleotide_sequence = "main",
       .schema =
          {.instance_name = "testInstanceName",
           .metadata =
              {
                 {.name = "testPrimaryKey", .type = ValueType::STRING},
                 {.name = "date_to_sort_by", .type = ValueType::DATE},
                 {.name = "not a pango lineage", .type = ValueType::STRING},
              },
           .primary_key = "testPrimaryKey",
           .date_to_sort_by = "date_to_sort_by",
           .partition_by = "not a pango lineage"}}
   );

   EXPECT_NO_THROW(ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml"););
}

TEST(ConfigRepository, givenMetadataToGenerateIndexForThatIsNotStringThenThrows) {
   const auto config_reader_mock = mockConfigReader(
      {.default_nucleotide_sequence = "main",
       .schema =
          {.instance_name = "testInstanceName",
           .metadata =
              {
                 {.name = "testPrimaryKey", .type = ValueType::STRING},
                 {.name = "indexed date", .type = ValueType::DATE, .generate_index = true},
              },
           .primary_key = "testPrimaryKey",
           .date_to_sort_by = std::nullopt,
           .partition_by = "testPrimaryKey"}}
   );

   EXPECT_THAT(
      [&config_reader_mock]() {
         ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml");
      },
      ThrowsMessage<ConfigException>(
         ::testing::HasSubstr("Metadata 'indexed date' generate_index is set, but generating an "
                              "index is only allowed for types STRING")
      )
   );
}
