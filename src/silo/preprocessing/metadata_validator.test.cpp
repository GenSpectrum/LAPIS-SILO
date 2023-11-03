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
      "main",
      {
         "testInstanceName",
         {
            {"gisaid_epi_isl", silo::config::ValueType::STRING},
            {"notInMetadata", silo::config::ValueType::PANGOLINEAGE},
            {"country", silo::config::ValueType::STRING},
         },
         "gisaid_epi_isl",
      }};

   const auto under_test = silo::preprocessing::MetadataValidator();
}

TEST(MetadataValidator, isValidMedataFileShouldReturnTrueWithValidMetadataFile) {
   const silo::config::DatabaseConfig valid_config{
      "main",
      {
         "testInstanceName",
         {
            {"gisaid_epi_isl", silo::config::ValueType::STRING},
            {"pango_lineage", silo::config::ValueType::PANGOLINEAGE},
            {"date", silo::config::ValueType::DATE},
            {"country", silo::config::ValueType::STRING},
         },
         "gisaid_epi_isl",
      }};

   const auto under_test = silo::preprocessing::MetadataValidator();

   EXPECT_NO_THROW(under_test.validateMedataFile(
      "testBaseData/exampleDataset/small_metadata_set.tsv", valid_config
   ));
}
