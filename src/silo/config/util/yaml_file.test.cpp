#include "config/backend/yaml_file.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "config/config_key_path.h"
#include "silo/common/fmt_formatters.h"

using silo::config::ConfigKeyPath;
using silo::config::YamlConfig;

// static std::string configKeyPathToString(const ConfigKeyPath& key_path);

// static ConfigKeyPath stringToConfigKeyPath(const std::string& key_path_string);

TEST(YamlConfig, simpleStringToConfigKeyPath) {
   auto under_test = YamlConfig::stringToConfigKeyPath("test");
   ASSERT_EQ(under_test, (ConfigKeyPath::tryFrom({{{"test"}}})));
}

TEST(YamlConfig, stringToConfigKeyPath1) {
   auto under_test = YamlConfig::stringToConfigKeyPath("api.port");
   ASSERT_EQ(under_test, (ConfigKeyPath::tryFrom({{"api"}, {"port"}})));
   ASSERT_NE(under_test, (ConfigKeyPath::tryFrom({{"api", "port"}})));
}

TEST(YamlConfig, stringToConfigKeyPath2) {
   auto under_test = YamlConfig::stringToConfigKeyPath("query.materializationCutoff");
   ASSERT_EQ(under_test, (ConfigKeyPath::tryFrom({{"query"}, {"materialization", "cutoff"}})));
}

TEST(YamlConfig, configKeyPathToString) {
   ASSERT_EQ(YamlConfig::configKeyPathToString(ConfigKeyPath::tryFrom({{"test"}}).value()), "test");
   ASSERT_EQ(
      YamlConfig::configKeyPathToString(
         (ConfigKeyPath::tryFrom({{"query"}, {"materialization", "cutoff"}})).value()
      ),
      "query.materializationCutoff"
   );
}

TEST(YamlConfig, validRoundTrip) {
   auto under_test =
      std::vector<std::string>{"test", "somethingElse.that.is.quiteLong", "a.2.3.4", "asd", "aa"};
   for (const auto& string : under_test) {
      ASSERT_EQ(
         YamlConfig::configKeyPathToString(YamlConfig::stringToConfigKeyPath(string)), string
      );
   }
}

TEST(YamlConfig, resolvesConfigKeyPath1) {
   auto under_test = YamlConfig::stringToConfigKeyPath("api.port");
   ASSERT_EQ(under_test, (ConfigKeyPath::tryFrom({{"api"}, {"port"}})));
   ASSERT_NE(under_test, (ConfigKeyPath::tryFrom({{"api", "port"}})));
}

TEST(YamlConfig, resolvesConfigKeyPath2) {
   auto under_test = YamlConfig::stringToConfigKeyPath("query.materializationCutoff");
   ASSERT_EQ(under_test, (ConfigKeyPath::tryFrom({{"query"}, {"materialization", "cutoff"}})));
}

TEST(YamlConfig, containsCorrectFieldsFromFlatYAML) {
   const auto under_test =
      YamlConfig::readFile("./testBaseData/test_preprocessing_config.yaml").getYamlFields();

   const std::unordered_map<ConfigKeyPath, YAML::Node> expected_result{
      {YamlConfig::stringToConfigKeyPath("inputDirectory"),
       YAML::Node{"./testBaseData/exampleDataset/"}},
      {YamlConfig::stringToConfigKeyPath("outputDirectory"), YAML::Node{"./output/"}},
      {YamlConfig::stringToConfigKeyPath("ndjsonInputFilename"), YAML::Node{"input_file.ndjson"}},
      {YamlConfig::stringToConfigKeyPath("lineageDefinitionsFilename"),
       YAML::Node{"lineage_definitions.yaml"}},
      {YamlConfig::stringToConfigKeyPath("referenceGenomeFilename"),
       YAML::Node{"reference_genomes.json"}},
   };

   for (const auto& [key, value] : expected_result) {
      ASSERT_TRUE(under_test.contains(key));
      ASSERT_EQ(under_test.at(key).as<std::string>(), value.as<std::string>());
   }
   for (const auto& [key, value] : under_test) {
      ASSERT_TRUE(expected_result.contains(key));
      ASSERT_EQ(expected_result.at(key).as<std::string>(), value.as<std::string>());
   }
}

TEST(YamlConfig, containsCorrectFieldsFromNestedYAML) {
   const auto under_test =
      YamlConfig::readFile("./testBaseData/test_runtime_config.yaml").getYamlFields();

   const std::unordered_map<ConfigKeyPath, YAML::Node> expected_result{
      {YamlConfig::stringToConfigKeyPath("dataDirectory"), YAML::Node{"test/directory"}},
      {YamlConfig::stringToConfigKeyPath("api.port"), YAML::Node{1234}},
   };

   for (const auto& [key, value] : expected_result) {
      ASSERT_TRUE(under_test.contains(key));
      ASSERT_EQ(under_test.at(key).as<std::string>(), value.as<std::string>());
   }
   for (const auto& [key, value] : under_test) {
      ASSERT_TRUE(expected_result.contains(key));
      ASSERT_EQ(expected_result.at(key).as<std::string>(), value.as<std::string>());
   }
}

TEST(YamlConfig, shouldThrowExceptionWhenConfigFileDoesNotExist) {
   EXPECT_THAT(
      []() { YamlConfig::readFile("testBaseData/does_not_exist.yaml"); },
      ThrowsMessage<std::runtime_error>(
         ::testing::HasSubstr("Could not open the YAML file: 'testBaseData/does_not_exist.yaml'")
      )
   );
}

TEST(YamlConfig, shouldThrowExceptionWhenConfigFileCannotBeParsed) {
   EXPECT_THAT(
      []() {
         YamlConfig::fromYAML("string", R"(
X
s:
)");
      },
      ThrowsMessage<std::runtime_error>(
         ::testing::HasSubstr("string does not contain valid YAML: yaml-cpp: error at line 3, "
                              "column 2: illegal map value")
      )
   );
}
