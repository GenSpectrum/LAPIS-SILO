#include "silo/preprocessing/metadata_info.h"

#include <algorithm>
#include <ranges>

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
      silo::preprocessing::MetadataInfo::validateMetadataFile(
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

   const auto raw_fields =
      silo::preprocessing::MetadataInfo::getMetadataFields(valid_config).getRawIdentifierStrings();
   ASSERT_TRUE(std::ranges::find(raw_fields, "gisaid_epi_isl") != raw_fields.end());
   ASSERT_TRUE(std::ranges::find(raw_fields, "pango_lineage") != raw_fields.end());
   ASSERT_TRUE(std::ranges::find(raw_fields, "date") != raw_fields.end());
   ASSERT_TRUE(std::ranges::find(raw_fields, "country") != raw_fields.end());

   const auto escaped_fields = silo::preprocessing::MetadataInfo::getMetadataFields(valid_config)
                                  .getEscapedIdentifierStrings();
   ASSERT_TRUE(std::ranges::find(escaped_fields, R"("gisaid_epi_isl")") != escaped_fields.end());
   ASSERT_TRUE(std::ranges::find(escaped_fields, R"("pango_lineage")") != escaped_fields.end());
   ASSERT_TRUE(std::ranges::find(escaped_fields, R"("date")") != escaped_fields.end());
   ASSERT_TRUE(std::ranges::find(escaped_fields, R"("country")") != escaped_fields.end());
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

   const auto raw_fields =
      silo::preprocessing::MetadataInfo::getMetadataFields(valid_config).getRawIdentifierStrings();
   ASSERT_TRUE(std::ranges::find(raw_fields, "gisaid_epi_isl") != raw_fields.end());
   ASSERT_TRUE(std::ranges::find(raw_fields, "pango_lineage") != raw_fields.end());
   ASSERT_TRUE(std::ranges::find(raw_fields, "date") != raw_fields.end());
   ASSERT_TRUE(std::ranges::find(raw_fields, "country") != raw_fields.end());

   const auto escaped_fields = silo::preprocessing::MetadataInfo::getMetadataFields(valid_config)
                                  .getEscapedIdentifierStrings();
   ASSERT_TRUE(std::ranges::find(escaped_fields, R"("gisaid_epi_isl")") != escaped_fields.end());
   ASSERT_TRUE(std::ranges::find(escaped_fields, R"("pango_lineage")") != escaped_fields.end());
   ASSERT_TRUE(std::ranges::find(escaped_fields, R"("date")") != escaped_fields.end());
   ASSERT_TRUE(std::ranges::find(escaped_fields, R"("country")") != escaped_fields.end());
}
