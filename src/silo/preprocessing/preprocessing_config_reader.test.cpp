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

   const std::string input_directory = "./testBaseData/exampleDataset/";
   const std::string output_directory = "./output/";
   ASSERT_EQ(config.getMetadataInputFilename(), input_directory + "small_metadata_set.tsv");
   ASSERT_EQ(
      config.getPangoLineageDefinitionFilename(), input_directory + "pangolineage_alias.json"
   );
   ASSERT_EQ(
      config.getNucPartitionFilename("dummy", 0, 0),
      output_directory + "partitions/nuc_dummy/P0_C0.zstdfasta"
   );
   ASSERT_EQ(
      config.getGenePartitionFilename("dummy2", 0, 0),
      output_directory + "partitions/gene_dummy2/P0_C0.zstdfasta"
   );
   ASSERT_EQ(
      config.getNucSortedPartitionFilename("dummy", 2, 1),
      output_directory + "partitions_sorted/nuc_dummy/P2_C1.zstdfasta"
   );
   ASSERT_EQ(
      config.getGeneSortedPartitionFilename("dummy", 2, 1),
      output_directory + "partitions_sorted/gene_dummy/P2_C1.zstdfasta"
   );
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

   const std::string input_directory = "./testBaseData/exampleDataset/";
   const std::string output_directory = "./output/";
   ASSERT_EQ(config.getMetadataInputFilename(), input_directory + "small_metadata_set.tsv");
   ASSERT_EQ(
      config.getPangoLineageDefinitionFilename(), input_directory + "pangolineage_alias.json"
   );

   ASSERT_EQ(config.getNucFilename("aligned"), input_directory + "aligned.fasta");
   ASSERT_EQ(
      config.getNucPartitionFilename("aligned", 0, 1),
      output_directory + "folder1/aligned/P0_C1.zstdfasta"
   );
   ASSERT_EQ(
      config.getNucSortedPartitionFilename("aligned", 2, 3),
      output_directory + "folder2/aligned/P2_C3.zstdfasta"
   );
}