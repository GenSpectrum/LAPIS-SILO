#include "silo/preprocessing/preprocessing_config_reader.h"

#include "silo/preprocessing/preprocessing_config.h"

#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>

using silo::preprocessing::PreprocessingConfig;
using silo::preprocessing::PreprocessingConfigReader;

TEST(PreprocessingConfigReader, shouldReadConfigWithCorrectParameters) {
   PreprocessingConfig config;

   ASSERT_NO_THROW(
      config =
         PreprocessingConfigReader().readConfig("./testBaseData/test_preprocessing_config.yaml")
   );

   const std::string input_directory = "./testBaseData/";
   const std::string output_directory = "./output/";
   ASSERT_EQ(config.input_directory, input_directory);
   ASSERT_EQ(config.metadata_file, input_directory + "small_metadata_set.tsv");
   ASSERT_EQ(config.pango_lineage_definition_file, input_directory + "pango_alias.txt");
   ASSERT_EQ(config.partition_folder, output_directory + "partitioned/");
   ASSERT_EQ(config.sequence_file, input_directory + "small_sequence_set.fasta");
   ASSERT_EQ(config.serialization_folder, output_directory + "serialized_state/");
   ASSERT_EQ(config.sorted_partition_folder, output_directory + "sorted_partitions/");
}

TEST(PreprocessingConfigReader, shouldThrowExceptionWhenConfigFileDoesNotExist) {
   ASSERT_THROW(
      PreprocessingConfigReader().readConfig("testBaseData/does_not_exist.yaml"), YAML::BadFile
   );
}