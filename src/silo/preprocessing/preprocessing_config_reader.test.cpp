#include "silo/preprocessing/preprocessing_config_reader.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "silo/preprocessing/preprocessing_config.h"

using silo::preprocessing::OptionalPreprocessingConfig;
using silo::preprocessing::PreprocessingConfig;
using silo::preprocessing::PreprocessingConfigReader;

TEST(PreprocessingConfigReader, shouldReadConfigWithCorrectParametersAndDefaults) {
   PreprocessingConfig config;

   ASSERT_NO_THROW(
      config = PreprocessingConfigReader()
                  .readConfig("./testBaseData/test_preprocessing_config.yaml")
                  .mergeValuesFromOrDefault(OptionalPreprocessingConfig())
   );

   const std::string input_directory = "./testBaseData/exampleDataset/";
   const std::string intermediate_directory = "./temp/";
   ASSERT_EQ(config.getMetadataInputFilename(), input_directory + "small_metadata_set.tsv");
   ASSERT_EQ(
      config.getPangoLineageDefinitionFilename(), input_directory + "pangolineage_alias.json"
   );
   ASSERT_EQ(
      config.getNucPartitionFilename("dummy", 0, 0),
      intermediate_directory + "partitions/nuc_dummy/P0_C0.zstdfasta"
   );
   ASSERT_EQ(
      config.getGenePartitionFilename("dummy2", 0, 0),
      intermediate_directory + "partitions/gene_dummy2/P0_C0.zstdfasta"
   );
   ASSERT_EQ(
      config.getNucSortedPartitionFilename("dummy", 2, 1),
      intermediate_directory + "partitions_sorted/nuc_dummy/P2_C1.zstdfasta"
   );
   ASSERT_EQ(
      config.getGeneSortedPartitionFilename("dummy", 2, 1),
      intermediate_directory + "partitions_sorted/gene_dummy/P2_C1.zstdfasta"
   );
}

TEST(PreprocessingConfigReader, shouldThrowExceptionWhenConfigFileDoesNotExist) {
   EXPECT_THAT(
      []() { PreprocessingConfigReader().readConfig("testBaseData/does_not_exist.yaml"); },
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr("Failed to read preprocessing config"))
   );
}

TEST(PreprocessingConfigReader, shouldReadConfigWithOverriddenDefaults) {
   PreprocessingConfig config;

   ASSERT_NO_THROW(
      config =
         PreprocessingConfigReader()
            .readConfig("./testBaseData/test_preprocessing_config_with_overridden_defaults.yaml")
            .mergeValuesFromOrDefault(OptionalPreprocessingConfig())
   );

   const std::string input_directory = "./testBaseData/exampleDataset/";
   const std::string intermediate_directory = "./output/overriddenTemp/";
   ASSERT_EQ(config.getMetadataInputFilename(), input_directory + "small_metadata_set.tsv");
   ASSERT_EQ(
      config.getPangoLineageDefinitionFilename(), input_directory + "pangolineage_alias.json"
   );

   ASSERT_EQ(config.getNucFilenameNoExtension("aligned"), input_directory + "aligned");
   ASSERT_EQ(
      config.getNucPartitionFilename("aligned", 0, 1),
      intermediate_directory + "folder1/aligned/P0_C1.zstdfasta"
   );
   ASSERT_EQ(
      config.getNucSortedPartitionFilename("aligned", 2, 3),
      intermediate_directory + "folder2/aligned/P2_C3.zstdfasta"
   );
   ASSERT_EQ(config.getOutputDirectory(), "./output/custom/");
}

TEST(OptionalPreprocessingConfig, givenLeftHandSideHasValueThenMergeTakesLeftHandSideValue) {
   silo::preprocessing::OptionalPreprocessingConfig left;
   left.gene_prefix = "leftTestPrefix_";
   auto right =
      PreprocessingConfigReader().readConfig("./testBaseData/test_preprocessing_config.yaml");
   right.gene_prefix = "rightTestPrefix_";

   const auto result = left.mergeValuesFromOrDefault(right);

   ASSERT_EQ(
      result.getGeneFilenameNoExtension("dummy"),
      std::filesystem::path("./testBaseData/exampleDataset/leftTestPrefix_dummy.fasta")
   );
}

TEST(OptionalPreprocessingConfig, givenLeftHandSideHasNotValueThenMergeTakesRightHandSideValue) {
   const silo::preprocessing::OptionalPreprocessingConfig left;
   auto right =
      PreprocessingConfigReader().readConfig("./testBaseData/test_preprocessing_config.yaml");
   right.gene_prefix = "rightTestPrefix_";

   const auto result = left.mergeValuesFromOrDefault(right);

   ASSERT_EQ(
      result.getGeneFilenameNoExtension("dummy"),
      std::filesystem::path("./testBaseData/exampleDataset/rightTestPrefix_dummy")
   );
}

TEST(OptionalPreprocessingConfig, givenNeitherSideHasValueThenMergeTakesDefaultValue) {
   const silo::preprocessing::OptionalPreprocessingConfig left;
   const auto right =
      PreprocessingConfigReader().readConfig("./testBaseData/test_preprocessing_config.yaml");

   const auto result = left.mergeValuesFromOrDefault(right);

   ASSERT_EQ(
      result.getGeneFilenameNoExtension("dummy"),
      std::filesystem::path("./testBaseData/exampleDataset/gene_dummy")
   );
}
