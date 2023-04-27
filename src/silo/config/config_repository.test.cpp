#include "silo/config/config_repository.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <filesystem>

#include "silo/config/config_exception.h"
#include "silo/config/database_config_reader.h"

class ConfigReaderMock : public silo::DatabaseConfigReader {
  public:
   MOCK_METHOD(
      (silo::DatabaseConfig),
      readConfig,
      (const std::filesystem::path&),
      (const override)
   );
};

TEST(ConfigRepository, shouldReadConfigWithoutErrors) {
   const ConfigReaderMock config_reader_mock;

   const silo::DatabaseConfig valid_config{
      "testInstanceName",
      {
         {"testPrimaryKey", silo::DatabaseMetadataType::STRING},
         {"metadata1", silo::DatabaseMetadataType::PANGOLINEAGE},
         {"metadata2", silo::DatabaseMetadataType::DATE},
      },
      "testPrimaryKey",
   };

   EXPECT_CALL(config_reader_mock, readConfig(testing::_))
      .WillRepeatedly(testing::Return(valid_config));

   ASSERT_NO_THROW(silo::ConfigRepository(config_reader_mock).readConfig("test.yaml"));
}

TEST(ConfigRepository, shouldThrowIfPrimaryKeyIsNotInMetadata) {
   const ConfigReaderMock config_reader_mock;

   const silo::DatabaseConfig config_without_primary_key{
      "testInstanceName",
      {
         {"notPrimaryKey", silo::DatabaseMetadataType::STRING},
      },
      "testPrimaryKey",
   };

   EXPECT_CALL(config_reader_mock, readConfig(testing::_))
      .WillRepeatedly(testing::Return(config_without_primary_key));

   ASSERT_THROW(
      silo::ConfigRepository(config_reader_mock).readConfig("test.yaml"), silo::ConfigException
   );
}

TEST(ConfigRepository, shouldThrowIfThereAreTwoMetadataWithTheSameName) {
   const ConfigReaderMock config_reader_mock;

   const silo::DatabaseConfig config_without_primary_key{
      "testInstanceName",
      {
         {"testPrimaryKey", silo::DatabaseMetadataType::STRING},
         {"sameName", silo::DatabaseMetadataType::PANGOLINEAGE},
         {"sameName", silo::DatabaseMetadataType::DATE},
      },
      "testPrimaryKey",
   };

   EXPECT_CALL(config_reader_mock, readConfig(testing::_))
      .WillRepeatedly(testing::Return(config_without_primary_key));

   ASSERT_THROW(
      silo::ConfigRepository(config_reader_mock).readConfig("test.yaml"), silo::ConfigException
   );
}

TEST(ConfigRepository, shouldReturnPrimaryKey) {
   const ConfigReaderMock config_reader_mock;

   const silo::DatabaseConfig valid_config{
      "testInstanceName",
      {
         {"testPrimaryKey", silo::DatabaseMetadataType::STRING},
         {"metadata1", silo::DatabaseMetadataType::PANGOLINEAGE},
         {"metadata2", silo::DatabaseMetadataType::DATE},
      },
      "testPrimaryKey",
   };

   EXPECT_CALL(config_reader_mock, readConfig(testing::_))
      .WillRepeatedly(testing::Return(valid_config));

   ASSERT_EQ(
      silo::ConfigRepository(config_reader_mock).getPrimaryKey("test.yaml"), "testPrimaryKey"
   );
}

TEST(ConfigRepository, shouldReturnMetadataByName) {
   const ConfigReaderMock config_reader_mock;

   const silo::DatabaseConfig valid_config{
      "testInstanceName",
      {
         {"testPrimaryKey", silo::DatabaseMetadataType::STRING},
         {"testMetadata", silo::DatabaseMetadataType::PANGOLINEAGE},
      },
      "testPrimaryKey",
   };

   EXPECT_CALL(config_reader_mock, readConfig(testing::_))
      .WillRepeatedly(testing::Return(valid_config));

   ASSERT_EQ(
      silo::ConfigRepository(config_reader_mock).getMetadata("test.yaml", "testMetadata").name,
      "testMetadata"
   );
   ASSERT_EQ(
      silo::ConfigRepository(config_reader_mock).getMetadata("test.yaml", "testMetadata").type,
      silo::DatabaseMetadataType::PANGOLINEAGE
   );
}

TEST(ConfigRepository, shouldThrowIfMetadataDoesNotExist) {
   const ConfigReaderMock config_reader_mock;

   const silo::DatabaseConfig valid_config{
      "testInstanceName",
      {
         {"testPrimaryKey", silo::DatabaseMetadataType::STRING},
         {"testMetadata", silo::DatabaseMetadataType::PANGOLINEAGE},
      },
      "testPrimaryKey",
   };

   EXPECT_CALL(config_reader_mock, readConfig(testing::_))
      .WillRepeatedly(testing::Return(valid_config));

   ASSERT_THROW(
      silo::ConfigRepository(config_reader_mock).getMetadata("test.yaml", "notExistingMetadata"),
      silo::ConfigException
   );
}