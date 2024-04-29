#include "silo/config/util/yaml_config.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "silo/config/preprocessing_config.h"

using silo::config::PreprocessingConfig;
using silo::config::YamlConfig;

TEST(YamlConfig, shouldReadConfigWithCorrectParametersAndDefaults) {
   PreprocessingConfig config;

   ASSERT_NO_THROW(config.overwrite(YamlConfig("./testBaseData/test_preprocessing_config.yaml")););

   const std::string input_directory = "./testBaseData/exampleDataset/";
   ASSERT_EQ(config.getMetadataInputFilename(), input_directory + "small_metadata_set.tsv");
   ASSERT_EQ(
      config.getPangoLineageDefinitionFilename(), input_directory + "pangolineage_alias.json"
   );
}

TEST(YamlConfig, shouldThrowExceptionWhenConfigFileDoesNotExist) {
   PreprocessingConfig config;
   EXPECT_THAT(
      [&config]() { config.overwrite(YamlConfig("testBaseData/does_not_exist.yaml")); },
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr("Failed to read preprocessing config"))
   );
}

TEST(YamlConfig, shouldReadConfigWithOverriddenDefaults) {
   PreprocessingConfig config;

   ASSERT_NO_THROW(config.overwrite(
      YamlConfig("./testBaseData/test_preprocessing_config_with_overridden_defaults.yaml")
   ););

   const std::string input_directory = "./testBaseData/exampleDataset/";
   ASSERT_EQ(config.getMetadataInputFilename(), input_directory + "small_metadata_set.tsv");
   ASSERT_EQ(
      config.getPangoLineageDefinitionFilename(), input_directory + "pangolineage_alias.json"
   );

   ASSERT_EQ(config.getNucFilenameNoExtension("aligned"), input_directory + "aligned");
   ASSERT_EQ(config.getOutputDirectory(), "./output/custom/");
}
