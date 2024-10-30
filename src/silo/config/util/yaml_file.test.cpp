#include "silo/config/util/yaml_file.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

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
   ASSERT_EQ(under_test.getString({{"ndjsonInputFilename"}}), "input_file.ndjson");
   ASSERT_EQ(under_test.getString({{"lineageDefinitionsFilename"}}), "lineage_definitions.yaml");
   ASSERT_EQ(under_test.getString({{"referenceGenomeFilename"}}), "reference_genomes.json");
}

TEST(YamlFile, getStringNulloptOnNotPresent) {
   const YamlFile under_test("./testBaseData/test_preprocessing_config.yaml");

   ASSERT_EQ(under_test.getString({{"a", "a"}}), std::nullopt);
   ASSERT_EQ(under_test.getString({{"again_not_present"}}), std::nullopt);
}
