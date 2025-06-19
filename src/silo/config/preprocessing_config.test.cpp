#include "silo/config/preprocessing_config.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "config/source/yaml_file.h"
#include "silo/preprocessing/preprocessing_exception.h"

using silo::config::PreprocessingConfig;
using silo::config::YamlFile;

TEST(PreprocessingConfig, shouldReadConfigWithCorrectParametersAndDefaults) {
   auto config = PreprocessingConfig::withDefaults();

   config.overwriteFrom(YamlFile::readFile("./testBaseData/test_preprocessing_config.yaml")
                           .verify(PreprocessingConfig::getConfigSpecification()));

   const std::string input_directory = "./testBaseData/exampleDataset/";
   ASSERT_TRUE(config.input_file.has_value());
   ASSERT_EQ(config.getInputFilePath(), input_directory + "input_file.ndjson");
   ASSERT_EQ(
      config.initialization_files.getLineageDefinitionsFilename(),
      input_directory + "lineage_definitions.yaml"
   );
   ASSERT_EQ(
      config.initialization_files.getphyloTreeFilename(), input_directory + "phylogenetic_tree.yaml"
   );
}

TEST(PreprocessingConfig, shouldReadConfigWithOverriddenDefaults) {
   auto config = PreprocessingConfig::withDefaults();

   config.overwriteFrom(YamlFile::fromYAML("inline", R"(
inputDirectory: "./testBaseData/exampleDataset/"
outputDirectory: "./output/custom/"
intermediateResultsDirectory: "./output/overriddenTemp/"
ndjsonInputFilename: "input_file.ndjson"
lineageDefinitionsFilename: "lineage_definitions.yaml"
phyloTreeFilename: "phylogenetic_tree.yaml"
referenceGenomeFilename: "reference_genomes.json"
preprocessingDatabaseLocation: "preprocessing.duckdb"
duckdbMemoryLimitInG: 8)")
                           .verify(PreprocessingConfig::getConfigSpecification()));

   const std::string input_directory = "./testBaseData/exampleDataset/";
   ASSERT_TRUE(config.input_file.has_value());
   ASSERT_EQ(config.getInputFilePath(), input_directory + "input_file.ndjson");
   ASSERT_EQ(
      config.initialization_files.getLineageDefinitionsFilename(),
      input_directory + "lineage_definitions.yaml"
   );
   ASSERT_EQ(
      config.initialization_files.getphyloTreeFilename(), input_directory + "phylogenetic_tree.yaml"
   );

   ASSERT_EQ(config.output_directory, "./output/custom/");
}

TEST(PreprocessingConfig, shouldThrowErrorWhenNdjsonInputFileNameIsNotSet) {
   auto config = PreprocessingConfig::withDefaults();
   EXPECT_THAT(
      [&config]() { config.validate(); },
      ThrowsMessage<silo::preprocessing::PreprocessingException>(
         ::testing::HasSubstr("'ndjsonInputFilename' must be specified as preprocessing option.")
      )
   );
}
