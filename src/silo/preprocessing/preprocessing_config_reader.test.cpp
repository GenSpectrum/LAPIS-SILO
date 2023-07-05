#include "silo/preprocessing/preprocessing_config_reader.h"

#include "silo/preprocessing/preprocessing_config.h"

#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>

using silo::preprocessing::PreprocessingConfig;
using silo::preprocessing::PreprocessingConfigReader;

TEST(PreprocessingConfigReader, shouldReadConfigWithCorrectParametersAndDefaults) {
   PreprocessingConfig config;

   ASSERT_NO_THROW(
      config =
         PreprocessingConfigReader().readConfig("./testBaseData/test_preprocessing_config.yaml")
   );

   const std::string input_directory = "./testBaseData/";
   const std::string output_directory = "./output/";
   ASSERT_EQ(config.input_directory, input_directory);
   ASSERT_EQ(config.metadata_file, input_directory + "small_metadata_set.tsv");
   ASSERT_EQ(config.pango_lineage_definition_file, input_directory + "pangolineage_alias.json");
   ASSERT_EQ(config.partition_folder, output_directory + "partitions/");
   ASSERT_EQ(config.serialization_folder, output_directory + "serialized_state/");
   ASSERT_EQ(config.sorted_partition_folder, output_directory + "partitions_sorted/");
}

TEST(PreprocessingConfigReader, shouldThrowExceptionWhenConfigFileDoesNotExist) {
   ASSERT_THROW(
      PreprocessingConfigReader().readConfig("testBaseData/does_not_exist.yaml"), YAML::BadFile
   );
}

TEST(PreprocessingConfigReader, shouldReadConfigWithOverriddenDefaults) {
   PreprocessingConfig config;

   ASSERT_NO_THROW(
      config = PreprocessingConfigReader().readConfig(
         "./testBaseData/test_preprocessing_config_with_overridden_defaults.yaml"
      )
   );

   const std::string input_directory = "./testBaseData/";
   const std::string output_directory = "./output/";
   ASSERT_EQ(config.input_directory, input_directory);
   ASSERT_EQ(config.metadata_file, input_directory + "small_metadata_set.tsv");
   ASSERT_EQ(config.pango_lineage_definition_file, input_directory + "pangolineage_alias.json");

   ASSERT_EQ(config.partition_folder, output_directory + "folder1/");
   ASSERT_EQ(config.sorted_partition_folder, output_directory + "folder2/");
   ASSERT_EQ(config.serialization_folder, output_directory + "folder3/");
}