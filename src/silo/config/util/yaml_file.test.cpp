#include "silo/config/util/yaml_file.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "silo/config/preprocessing_config.h"

using silo::config::PreprocessingConfig;
using silo::config::YamlFile;

TEST(YamlFile, canCorrectlyCheckForPresentPropertiesCaseSensitively) {
   const YamlFile under_test("./testBaseData/test_preprocessing_config.yaml");

   ASSERT_EQ(under_test.hasProperty({{"inputDirectory"}}), true);
   ASSERT_EQ(under_test.hasProperty({{"INPUTDIRECTORY"}}), false);
}

TEST(YamlFile, canCorrectlyCheckForNonPresentProperties) {
   const YamlFile under_test("./testBaseData/test_preprocessing_config.yaml");

   ASSERT_EQ(under_test.hasProperty({{"a"}}), false);
}

TEST(YamlFile, getStringGetsCorrectField) {
   const YamlFile under_test("./testBaseData/test_preprocessing_config.yaml");

   ASSERT_EQ(under_test.getString({{"inputDirectory"}}), "./testBaseData/exampleDataset/");
}

TEST(YamlFile, getStringGetsCorrectFieldsRepeatedly) {
   const YamlFile under_test("./testBaseData/test_preprocessing_config.yaml");

   ASSERT_EQ(under_test.getString({{"inputDirectory"}}), "./testBaseData/exampleDataset/");
   ASSERT_EQ(under_test.getString({{"outputDirectory"}}), "./output/");
   ASSERT_EQ(under_test.getString({{"metadataFilename"}}), "small_metadata_set.tsv");
   ASSERT_EQ(under_test.getString({{"lineageDefinitionsFilename"}}), "lineage_definitions.yaml");
   ASSERT_EQ(under_test.getString({{"referenceGenomeFilename"}}), "reference_genomes.json");
}

TEST(YamlFile, getStringNulloptOnNotPresent) {
   const YamlFile under_test("./testBaseData/test_preprocessing_config.yaml");

   ASSERT_EQ(under_test.getString({{"a", "a"}}), std::nullopt);
   ASSERT_EQ(under_test.getString({{"again_not_present"}}), std::nullopt);
}

TEST(YamlFile, shouldReadConfigWithCorrectParametersAndDefaults) {
   PreprocessingConfig config;

   ASSERT_NO_THROW(config.overwrite(YamlFile("./testBaseData/test_preprocessing_config.yaml")););

   const std::string input_directory = "./testBaseData/exampleDataset/";
   ASSERT_EQ(config.getMetadataInputFilename(), input_directory + "small_metadata_set.tsv");
   ASSERT_EQ(config.getLineageDefinitionsFilename(), input_directory + "lineage_definitions.yaml");
}

TEST(YamlFile, shouldThrowExceptionWhenConfigFileDoesNotExist) {
   PreprocessingConfig config;
   EXPECT_THAT(
      [&config]() { config.overwrite(YamlFile("testBaseData/does_not_exist.yaml")); },
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr("Failed to read preprocessing config"))
   );
}

TEST(YamlFile, shouldReadConfigWithOverriddenDefaults) {
   PreprocessingConfig config;

   ASSERT_NO_THROW(config.overwrite(
      YamlFile("./testBaseData/test_preprocessing_config_with_overridden_defaults.yaml")
   ););

   const std::string input_directory = "./testBaseData/exampleDataset/";
   ASSERT_EQ(config.getMetadataInputFilename(), input_directory + "small_metadata_set.tsv");
   ASSERT_EQ(config.getLineageDefinitionsFilename(), input_directory + "lineage_definitions.yaml");

   ASSERT_EQ(config.getNucFilenameNoExtension(0), input_directory + "0");
   ASSERT_EQ(config.getOutputDirectory(), "./output/custom/");
}
