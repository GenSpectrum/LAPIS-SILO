#include "silo/preprocessing/metadata.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "silo/preprocessing/preprocessing_exception.h"

TEST(MetadataReader, readColumnOfMetadataFile) {
   const auto test_column = silo::preprocessing::MetadataReader::getColumn(
      "testBaseData/small_metadata_set.tsv", "gisaid_epi_isl"
   );

   ASSERT_EQ(test_column.size(), 100);
   ASSERT_EQ(test_column[0], "EPI_ISL_1408408");
}

TEST(MetadataReader, shouldThrowExceptionWhenColumnDoesNotExist) {
   EXPECT_THAT(
      []() {
         silo::preprocessing::MetadataReader::getColumn(
            "testBaseData/small_metadata_set.tsv", "columnThatDoesNotExist"
         );
      },
      ThrowsMessage<silo::PreprocessingException>(
         ::testing::HasSubstr("Can't find a column named columnThatDoesNotExist")
      )
   );
}

TEST(MetadataReader, shouldThrowExceptionWhenFileDoesNotExist) {
   EXPECT_THAT(
      []() {
         silo::preprocessing::MetadataReader::getColumn("file.does.not.exist", "gisaid_epi_isl");
      },
      ThrowsMessage<silo::PreprocessingException>(
         ::testing::HasSubstr("Failed to read metadata file 'file.does.not.exist'")
      )
   );
}