#include "silo/preprocessing/metadata_validator.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/config/config_repository.h"
#include "silo/preprocessing/metadata.h"
#include "silo/preprocessing/preprocessing_exception.h"

class ConfigRepositoryMock : public silo::config::ConfigRepository {
  public:
   MOCK_METHOD(
      (silo::config::DatabaseConfig),
      getValidatedConfig,
      (const std::filesystem::path&),
      (const)
   );
};

TEST(
   MetadataValidator,
   isValidMedataFileShouldReturnFalseWhenOneConfigCoulmnIsNotPresentInMetadataFile
) {
   const ConfigRepositoryMock config_repository;

   const silo::config::DatabaseConfig some_config_with_one_column_not_in_metadata{
      "testInstanceName",
      {
         {"gisaid_epi_isl", silo::config::DatabaseMetadataType::STRING},
         {"notInMetadata", silo::config::DatabaseMetadataType::PANGOLINEAGE},
         {"country", silo::config::DatabaseMetadataType::STRING},
      },
      "gisaid_epi_isl",
   };

   EXPECT_CALL(config_repository, getValidatedConfig(testing::_))
      .WillRepeatedly(testing::Return(some_config_with_one_column_not_in_metadata));

   const auto under_test = silo::preprocessing::MetadataValidator(config_repository);

   EXPECT_THAT(
      [=]() { under_test.validateMedataFile("testBaseData/small_metadata_set.tsv", "someConfig"); },
      ThrowsMessage<silo::PreprocessingException>(
         ::testing::HasSubstr("Metadata file does not contain column: notInMetadata")
      )
   );
}

TEST(MetadataValidator, isValidMedataFileShouldReturnTrueWithValidMetadataFile) {
   const ConfigRepositoryMock config_repository;

   const silo::config::DatabaseConfig valid_config{
      "testInstanceName",
      {
         {"gisaid_epi_isl", silo::config::DatabaseMetadataType::STRING},
         {"pango_lineage", silo::config::DatabaseMetadataType::PANGOLINEAGE},
         {"date", silo::config::DatabaseMetadataType::DATE},
         {"country", silo::config::DatabaseMetadataType::STRING},
      },
      "gisaid_epi_isl",
   };

   EXPECT_CALL(config_repository, getValidatedConfig(testing::_))
      .WillRepeatedly(testing::Return(valid_config));

   const auto under_test = silo::preprocessing::MetadataValidator(config_repository);

   EXPECT_NO_THROW(
      under_test.validateMedataFile("testBaseData/small_metadata_set.tsv", "someConfig")
   );
}
