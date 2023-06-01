#include "silo/preprocessing/metadata_validator.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/config/config_repository.h"
#include "silo/preprocessing/metadata.h"
#include "silo/preprocessing/preprocessing_exception.h"

TEST(
   MetadataValidator,
   isValidMedataFileShouldReturnFalseWhenOneConfigCoulmnIsNotPresentInMetadataFile
) {
   const silo::config::DatabaseConfig some_config_with_one_column_not_in_metadata{
      "testInstanceName",
      {
         {"gisaid_epi_isl", silo::config::DatabaseMetadataType::STRING},
         {"notInMetadata", silo::config::DatabaseMetadataType::PANGOLINEAGE},
         {"country", silo::config::DatabaseMetadataType::STRING},
      },
      "gisaid_epi_isl",
   };

   const auto under_test = silo::preprocessing::MetadataValidator();

   EXPECT_THAT(
      [=]() {
         under_test.validateMedataFile(
            "testBaseData/small_metadata_set.tsv", some_config_with_one_column_not_in_metadata
         );
      },
      ThrowsMessage<silo::PreprocessingException>(
         ::testing::HasSubstr("Metadata file does not contain column: notInMetadata")
      )
   );
}

TEST(MetadataValidator, isValidMedataFileShouldReturnTrueWithValidMetadataFile) {
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

   const auto under_test = silo::preprocessing::MetadataValidator();

   EXPECT_NO_THROW(
      under_test.validateMedataFile("testBaseData/small_metadata_set.tsv", valid_config)
   );
}
