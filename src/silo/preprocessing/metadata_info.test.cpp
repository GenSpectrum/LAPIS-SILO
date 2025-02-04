#include "silo/preprocessing/metadata_info.h"

#include <algorithm>
#include <ranges>

#include <gtest/gtest.h>

#include "silo/preprocessing/preprocessing_exception.h"

TEST(
   MetadataInfo,
   isValidMedataFileShouldReturnFalseWhenOneConfigCoulmnIsNotPresentInMetadataFile
) {
   const silo::config::DatabaseConfig some_config_with_one_column_not_in_metadata =
      silo::config::DatabaseConfig::getValidatedConfig(
         R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "gisaidEpiIsl"
      type: "string"
    - name: "notInMetadata"
      type: "string"
    - name: "country"
      type: "string"
  primaryKey: "gisaidEpiIsl"
)"
      );

   EXPECT_THROW(
      silo::preprocessing::MetadataInfo::validateMetadataFile(
         "testBaseData/exampleDataset/small_metadata_set.tsv",
         some_config_with_one_column_not_in_metadata
      ),
      silo::preprocessing::PreprocessingException
   );
}
