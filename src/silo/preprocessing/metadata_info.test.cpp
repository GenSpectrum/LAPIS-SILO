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
