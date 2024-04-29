#include "silo/preprocessing/metadata_info.h"

#include <gtest/gtest.h>

#include "silo/config/util/config_repository.h"
#include "silo/preprocessing/preprocessing_exception.h"

TEST(
   MetadataInfo,
   isValidMedataFileShouldReturnFalseWhenOneConfigCoulmnIsNotPresentInMetadataFile
) {
   const silo::config::DatabaseConfig some_config_with_one_column_not_in_metadata{
      .default_nucleotide_sequence = "main",
      .schema =
         {
            .instance_name = "testInstanceName",
            .metadata =
               {
                  {.name = "gisaid_epi_isl", .type = silo::config::ValueType::STRING},
                  {.name = "notInMetadata", .type = silo::config::ValueType::PANGOLINEAGE},
                  {.name = "country", .type = silo::config::ValueType::STRING},
               },
            .primary_key = "gisaid_epi_isl",
         }
   };

   EXPECT_THROW(
      silo::preprocessing::MetadataInfo::validateFromMetadataFile(
         "testBaseData/exampleDataset/small_metadata_set.tsv",
         some_config_with_one_column_not_in_metadata
      ),
      silo::preprocessing::PreprocessingException
   );
}

TEST(MetadataInfo, isValidMedataFileShouldReturnTrueWithValidMetadataFile) {
   const silo::config::DatabaseConfig valid_config{
      .default_nucleotide_sequence = "main",
      .schema =
         {
            .instance_name = "testInstanceName",
            .metadata =
               {
                  {.name = "gisaid_epi_isl", .type = silo::config::ValueType::STRING},
                  {.name = "pango_lineage", .type = silo::config::ValueType::PANGOLINEAGE},
                  {.name = "date", .type = silo::config::ValueType::DATE},
                  {.name = "country", .type = silo::config::ValueType::STRING},
               },
            .primary_key = "gisaid_epi_isl",
         }
   };

   const auto fields = silo::preprocessing::MetadataInfo::validateFromMetadataFile(
                          "testBaseData/exampleDataset/small_metadata_set.tsv", valid_config
   )
                          .getMetadataFields();
   ASSERT_TRUE(std::find(fields.begin(), fields.end(), R"("gisaid_epi_isl")") != fields.end());
   ASSERT_TRUE(std::find(fields.begin(), fields.end(), R"("pango_lineage")") != fields.end());
   ASSERT_TRUE(std::find(fields.begin(), fields.end(), R"("date")") != fields.end());
   ASSERT_TRUE(std::find(fields.begin(), fields.end(), R"("country")") != fields.end());
}

TEST(MetadataInfo, shouldValidateCorrectNdjsonInputFile) {
   const silo::config::DatabaseConfig valid_config{
      .default_nucleotide_sequence = "main",
      .schema =
         {
            .instance_name = "testInstanceName",
            .metadata =
               {
                  {.name = "gisaid_epi_isl", .type = silo::config::ValueType::STRING},
                  {.name = "pango_lineage", .type = silo::config::ValueType::PANGOLINEAGE},
                  {.name = "date", .type = silo::config::ValueType::DATE},
                  {.name = "country", .type = silo::config::ValueType::STRING},
               },
            .primary_key = "gisaid_epi_isl",
         }
   };

   const auto fields = silo::preprocessing::MetadataInfo::validateFromNdjsonFile(
                          "testBaseData/exampleDatasetAsNdjson/input_file.ndjson", valid_config
   )
                          .getMetadataFields();

   ASSERT_TRUE(std::find(fields.begin(), fields.end(), R"("gisaid_epi_isl")") != fields.end());
   ASSERT_TRUE(std::find(fields.begin(), fields.end(), R"("pango_lineage")") != fields.end());
   ASSERT_TRUE(std::find(fields.begin(), fields.end(), R"("date")") != fields.end());
   ASSERT_TRUE(std::find(fields.begin(), fields.end(), R"("country")") != fields.end());
}
