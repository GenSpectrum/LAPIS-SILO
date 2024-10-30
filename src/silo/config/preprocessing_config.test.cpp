#include "silo/config/preprocessing_config.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/config/util/yaml_file.h"
#include "silo/preprocessing/preprocessing_exception.h"

using silo::config::PreprocessingConfig;
using silo::config::YamlFile;

TEST(PreprocessingConfig, shouldReadConfigWithCorrectParametersAndDefaults) {
   PreprocessingConfig config;

   ASSERT_NO_THROW(config.overwrite(YamlFile("./testBaseData/test_preprocessing_config.yaml")););

   const std::string input_directory = "./testBaseData/exampleDataset/";
   ASSERT_EQ(config.getNdjsonInputFilename(), input_directory + "input_file.ndjson");
   ASSERT_EQ(config.getLineageDefinitionsFilename(), input_directory + "lineage_definitions.yaml");
}

TEST(PreprocessingConfig, shouldThrowExceptionWhenConfigFileDoesNotExist) {
   PreprocessingConfig config;
   EXPECT_THAT(
      [&config]() { config.overwrite(YamlFile("testBaseData/does_not_exist.yaml")); },
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr("Failed to read preprocessing config"))
   );
}

TEST(PreprocessingConfig, shouldReadConfigWithOverriddenDefaults) {
   PreprocessingConfig config;

   ASSERT_NO_THROW(config.overwrite(
      YamlFile("./testBaseData/test_preprocessing_config_with_overridden_defaults.yaml")
   ););

   const std::string input_directory = "./testBaseData/exampleDataset/";
   ASSERT_EQ(config.getNdjsonInputFilename(), input_directory + "input_file.ndjson");
   ASSERT_EQ(config.getLineageDefinitionsFilename(), input_directory + "lineage_definitions.yaml");
   ASSERT_EQ(config.getDuckdbMemoryLimitInG(), 8);
   ASSERT_EQ(config.getPreprocessingDatabaseLocation(), "preprocessing.duckdb");

   ASSERT_EQ(config.getOutputDirectory(), "./output/custom/");
}

TEST(PreprocessingConfig, shouldThrowErrorWhenNdjsonInputFileNameIsNotSet) {
   PreprocessingConfig config;

   EXPECT_THAT(
      [&config]() { config.validate(); },
      ThrowsMessage<silo::preprocessing::PreprocessingException>(
         ::testing::HasSubstr("ndjsonInputFilename must be specified as preprocessing option.")
      )
   );
}
