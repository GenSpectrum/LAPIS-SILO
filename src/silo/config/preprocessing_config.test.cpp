#include "silo/config/preprocessing_config.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "config/backend/yaml_file.h"
#include "silo/preprocessing/preprocessing_exception.h"

using silo::config::PreprocessingConfig;
using silo::config::YamlConfig;

TEST(PreprocessingConfig, shouldReadConfigWithCorrectParametersAndDefaults) {
   PreprocessingConfig config;

   ASSERT_NO_THROW(
      config.overwriteFrom(YamlConfig::readFile("./testBaseData/test_preprocessing_config.yaml")
                              .verify(PreprocessingConfig::getConfigSpecification()))
   );

   const std::string input_directory = "./testBaseData/exampleDataset/";
   ASSERT_EQ(config.getNdjsonInputFilename(), input_directory + "input_file.ndjson");
   ASSERT_EQ(config.getLineageDefinitionsFilename(), input_directory + "lineage_definitions.yaml");
}

TEST(PreprocessingConfig, shouldReadConfigWithOverriddenDefaults) {
   PreprocessingConfig config;

   ASSERT_NO_THROW(config.overwriteFrom(
      YamlConfig::readFile("./testBaseData/test_preprocessing_config_with_overridden_defaults.yaml")
         .verify(PreprocessingConfig::getConfigSpecification())
   ););

   const std::string input_directory = "./testBaseData/exampleDataset/";
   ASSERT_EQ(config.getNdjsonInputFilename(), input_directory + "input_file.ndjson");
   ASSERT_EQ(config.getLineageDefinitionsFilename(), input_directory + "lineage_definitions.yaml");
   ASSERT_EQ(config.getDuckdbMemoryLimitInG(), 8);
   ASSERT_EQ(config.preprocessing_database_location, "preprocessing.duckdb");

   ASSERT_EQ(config.output_directory, "./output/custom/");
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
