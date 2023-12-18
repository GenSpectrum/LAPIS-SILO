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
   ASSERT_EQ(config.getMetadataInputFilename(), input_directory + "small_metadata_set.tsv");
   ASSERT_EQ(
      config.getPangoLineageDefinitionFilename(), input_directory + "pangolineage_alias.json"
   );

   ASSERT_EQ(config.getNucFilenameNoExtension("aligned"), input_directory + "aligned");
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
      std::filesystem::path("./testBaseData/exampleDataset/leftTestPrefix_dummy")
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
