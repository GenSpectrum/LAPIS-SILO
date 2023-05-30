#include "silo/config/config_repository.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <filesystem>

#include "silo/config/config_exception.h"
#include "silo/config/database_config_reader.h"

using silo::config::ConfigException;
using silo::config::ConfigRepository;
using silo::config::DatabaseConfig;
using silo::config::DatabaseMetadataType;

class ConfigReaderMock : public silo::config::DatabaseConfigReader {
  public:
   MOCK_METHOD((DatabaseConfig), readConfig, (const std::filesystem::path&), (const override));
};

TEST(ConfigRepository, shouldReadConfigWithoutErrors) {
   const ConfigReaderMock config_reader_mock;

   const DatabaseConfig valid_config{
      "testInstanceName",
      {
         {"testPrimaryKey", DatabaseMetadataType::STRING},
         {"metadata1", DatabaseMetadataType::PANGOLINEAGE},
         {"metadata2", DatabaseMetadataType::DATE},
      },
      "testPrimaryKey",
   };

   EXPECT_CALL(config_reader_mock, readConfig(testing::_))
      .WillRepeatedly(testing::Return(valid_config));

   ASSERT_NO_THROW(ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml"));
}

TEST(ConfigRepository, shouldThrowIfPrimaryKeyIsNotInMetadata) {
   const ConfigReaderMock config_reader_mock;

   const DatabaseConfig config_without_primary_key{
      "testInstanceName",
      {
         {"notPrimaryKey", DatabaseMetadataType::STRING},
      },
      "testPrimaryKey",
   };

   EXPECT_CALL(config_reader_mock, readConfig(testing::_))
      .WillRepeatedly(testing::Return(config_without_primary_key));

   ASSERT_THROW(
      ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml"), ConfigException
   );
}

TEST(ConfigRepository, shouldThrowIfThereAreTwoMetadataWithTheSameName) {
   const ConfigReaderMock config_reader_mock;

   const DatabaseConfig config_without_primary_key{
      "testInstanceName",
      {
         {"testPrimaryKey", DatabaseMetadataType::STRING},
         {"sameName", DatabaseMetadataType::PANGOLINEAGE},
         {"sameName", DatabaseMetadataType::DATE},
      },
      "testPrimaryKey",
   };

   EXPECT_CALL(config_reader_mock, readConfig(testing::_))
      .WillRepeatedly(testing::Return(config_without_primary_key));

   ASSERT_THROW(
      ConfigRepository(config_reader_mock).getValidatedConfig("test.yaml"), ConfigException
   );
}
