#include "silo/preprocessing/preprocessing.h"

#include <fstream>
#include <functional>
#include <vector>

#include <fmt/ranges.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "config/source/yaml_file.h"
#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/database_info.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/query_engine/query.h"
#include "silo/query_engine/query_plan.h"

namespace {
using silo::ReferenceGenomes;
using silo::common::LineageTreeAndIdMap;
using silo::config::DatabaseConfig;
using silo::config::PreprocessingConfig;
using silo::config::ValueType;
using silo::preprocessing::PreprocessingException;

struct Error {
   std::string error_message;
};

struct Success {
   size_t expected_sequence_count;
   std::string query;
   nlohmann::json expected_query_result;
};

template <typename Assertion>
struct Scenario {
   std::string test_name;
   std::function<std::vector<nlohmann::json>()> input_data;
   std::string database_config;
   std::string reference_genomes;
   std::map<std::filesystem::path, std::string> lineage_trees;
   Assertion assertion;
};

template <typename Assertion>
silo::config::PreprocessingConfig prepareInputDirAndPreprocessorForScenario(
   Scenario<Assertion> scenario
) {
   auto now = std::chrono::system_clock::now();
   auto duration = now.time_since_epoch();
   auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
   const std::filesystem::path input_directory =
      fmt::format("test_{}_{}", scenario.test_name, millis);
   std::filesystem::create_directories(input_directory);

   std::ofstream database_config_file(input_directory / "database_config.yaml");
   database_config_file << scenario.database_config;
   database_config_file.close();

   std::ofstream reference_genomes_file(input_directory / "reference_genomes.json");
   reference_genomes_file << scenario.reference_genomes;
   reference_genomes_file.close();

   for (const auto& [filename, lineage_tree] : scenario.lineage_trees) {
      // Assert that 'filename' is a filename and not a path
      SILO_ASSERT_EQ(filename.filename(), filename);
      std::ofstream lineage_definition_file(input_directory / filename);
      lineage_definition_file << lineage_tree;
      lineage_definition_file.close();
   }

   auto config_with_input_dir = PreprocessingConfig::withDefaults();
   config_with_input_dir.initialization_files.directory = input_directory;
   config_with_input_dir.input_file = "input.json";
   if (not scenario.lineage_trees.empty()) {
      auto keys = scenario.lineage_trees | std::views::keys |
                  std::views::transform([](const std::filesystem::path& p) { return p.string(); });
      config_with_input_dir.initialization_files.lineage_definition_files =
         std::vector<std::string>(keys.begin(), keys.end());
   }
   config_with_input_dir.validate();

   std::ofstream file(config_with_input_dir.getInputFilePath().value());

   if (!file.is_open()) {
      throw std::runtime_error("Could not open file for writing ndjson data");
   }
   for (const auto& line : scenario.input_data()) {
      file << line.dump() << std::endl;
   }
   file.close();

   return config_with_input_dir;
}

template <typename Assertion>
std::string printTestName(const ::testing::TestParamInfo<Scenario<Assertion>>& info) {
   std::string name = "Dir_" + info.param.test_name;
   std::ranges::replace(name, '/', '_');
   return name;
}

const Scenario<Success> NDJSON_FILE_WITH_MISSING_SEGMENTS_AND_GENES = {
   .test_name = "NDJSON_FILE_WITH_MISSING_SEGMENTS_AND_GENES",
   .input_data =
      []() {
         std::vector<nlohmann::json> result;
         result.push_back(nlohmann::json::parse(R"({
"accessionVersion": "1.1",
"country": "Switzerland",
"main": {"sequence": "NNACTGNN", "insertions": ["123:ACTG"]},
"secondSegment": null,
"someLongGene": {"sequence": "ACDEFGHIKLMNPQRSTVWYBZX-*", "insertions": ["123:RNRNRN"]},
"someShortGene": {"sequence": "MADS", "insertions": ["123:RN"]},
"unaligned_main": "ATTAAAGGTTTATACCTTCCCAGGTAACAAACCAACCAACTTTCGATCT",
"unaligned_secondSegment": null
})"));
         result.push_back(nlohmann::json::parse(R"({
"accessionVersion": "1.3",
"country": "Germany",
"main": null,
"secondSegment": null,
"someLongGene": null,
"someShortGene": null,
"unaligned_main": null,
"unaligned_secondSegment": null
})"));
         return result;
      },
   .database_config =
      R"(
schema:
  instanceName: "Test"
  metadata:
    - name: "accessionVersion"
      type: "string"
    - name: "country"
      type: "string"
  primaryKey: "accessionVersion"
)",
   .reference_genomes = R"(
{
  "nucleotideSequences": [
    {
      "name": "main",
      "sequence": "ATTAAAGGTTTATACCTTCCCAGGTAACAAACCAACCAACTTTCGATCT"
    },
    {
      "name": "secondSegment",
      "sequence": "AAAAAAAAAAAAAAAA"
    }
  ],
  "genes": [
    {
      "name": "someLongGene",
      "sequence": "AAAAAAAAAAAAAAAAAAAAAAAAA"
    },
    {
      "name": "someShortGene",
      "sequence": "MADS"
    }
  ]
})",
   .assertion{
      .expected_sequence_count = 2,
      .query = R"(
         {
            "action": {
              "type": "FastaAligned",
              "sequenceNames": ["someShortGene", "secondSegment"],
              "orderByFields": ["accessionVersion"],
              "additionalFields": ["country"]
            },
            "filterExpression": {
               "type": "True"
            }
         }
      )",
      .expected_query_result = nlohmann::json::parse(R"(
   [{
      "accessionVersion": "1.1",
      "country": "Switzerland",
      "someShortGene": "MADS",
      "secondSegment": "NNNNNNNNNNNNNNNN"
   },
   {
      "accessionVersion": "1.3",
      "country": "Germany",
      "someShortGene": "XXXX",
      "secondSegment": "NNNNNNNNNNNNNNNN"
   }])")
   }
};

const Scenario<Success> NDJSON_WITH_SQL_KEYWORD_AS_FIELD = {
   .test_name = "NDJSON_WITH_SQL_KEYWORD_AS_FIELD",
   .input_data =
      []() {
         std::vector<nlohmann::json> result;
         result.push_back(nlohmann::json::parse(R"({
"primaryKey": "1.1",
"group": "dummyValue",
"main": {"sequence": "A", "insertions": []},
"mainGene": {"sequence": "A*", "insertions": []},
"unaligned_main": "A"
})"));
         result.push_back(nlohmann::json::parse(R"({
"primaryKey": "1.2",
"group": null,
"main": null,
"mainGene": null,
"unaligned_main": null
})"));
         return result;
      },
   .database_config =
      R"(
schema:
   instanceName: "Test"
   metadata:
   - name: "primaryKey"
     type: "string"
   - name: "group"
     type: "string"
   primaryKey: "primaryKey"
)",
   .reference_genomes = R"({
  "nucleotideSequences": [
    {
      "name": "main",
      "sequence": "A"
    }
  ],
  "genes": [
    {
      "name": "mainGene",
      "sequence": "A*"
    }
  ]
})",
   .assertion{
      .expected_sequence_count = 2,
      .query = R"(
{
   "action": {
      "type": "Aggregated",
      "groupByFields": ["group"],
      "orderByFields": ["group"]
   },
   "filterExpression": {
      "type": "True"
   }
}
)",
      .expected_query_result = nlohmann::json::parse(
         R"([
         {"count": 1, "group": null},
         {"count": 1, "group": "dummyValue"}
   ])"
      )
   }
};

const Scenario<Success> NDJSON_WITH_NUMERIC_NAMES = {
   .test_name = "NDJSON_WITH_NUMERIC_NAMES",
   .input_data =
      []() {
         std::vector<nlohmann::json> result;
         result.push_back(nlohmann::json::parse(R"({
"2": "google.com",
"accessionVersion": "1.1",
"main": {"sequence": "A", "insertions": []},
"3": {"sequence": "AA", "insertions": []},
"someGene": {"sequence": "AA*", "insertions": []},
"4": {"sequence": "AA*", "insertions": []},
"unaligned_main": "A",
"unaligned_3": "AA"
})"));
         result.push_back(nlohmann::json::parse(R"({
"2": null,
"accessionVersion": "1.3",
"main": null,
"3": null,
"someGene": null,
"4": null,
"unaligned_main": null,
"unaligned_3": null
})"));
         return result;
      },
   .database_config =
      R"(
schema:
  instanceName: "Test"
  metadata:
    - name: "accessionVersion"
      type: "string"
    - name: "2"
      type: "string"
  primaryKey: "accessionVersion"
)",
   .reference_genomes = R"(
{
  "nucleotideSequences": [
    {
      "name": "main",
      "sequence": "A"
    },
    {
      "name": "3",
      "sequence": "AA"
    }
  ],
  "genes": [
    {
      "name": "someGene",
      "sequence": "AA*"
    },
    {
      "name": "4",
      "sequence": "A*"
    }
  ]
})",
   .assertion{
      .expected_sequence_count = 2,
      .query = R"(
      {
         "action": {
            "type": "Aggregated",
            "groupByFields": ["2"],
            "orderByFields": ["2"]
         },
         "filterExpression": {
            "type": "True"
         }
      }
   )",
      .expected_query_result = nlohmann::json::parse(
         R"([
         {"count": 1, "2": null},
         {"count": 1, "2": "google.com"}
   ])"
      )
   }
};

const Scenario<Success> EMPTY_INPUT_NDJSON = {
   .test_name = "EMPTY_INPUT_NDJSON",
   .input_data = []() { return std::vector<nlohmann::json>{}; },
   .database_config =
      R"(
schema:
  instanceName: "Test"
  metadata:
    - name: "accessionVersion"
      type: "string"
    - name: "2"
      type: "string"
      generateIndex: true
      generateLineageIndex: test_lineage_definition.yaml
  primaryKey: "accessionVersion"
)",
   .reference_genomes = R"(
{
  "nucleotideSequences": [
    {
      "name": "main",
      "sequence": "A"
    },
    {
      "name": "3",
      "sequence": "AA"
    }
  ],
  "genes": [
    {
      "name": "someGene",
      "sequence": "AA*"
    },
    {
      "name": "4",
      "sequence": "A*"
    }
  ]
})",
   .lineage_trees = {{"test_lineage_definition.yaml", "main: ~\n"}},
   .assertion{
      .expected_sequence_count = 0,
      .query = R"(
      {
         "action": {
           "type": "Details"
         },
         "filterExpression": {
            "type": "True"
         }
      }
   )",
      .expected_query_result = nlohmann::json::parse(R"(
[])")
   }
};

const Scenario<Success> EMPTY_INPUT_NDJSON_UNPARTITIONED = {
   .test_name = "EMPTY_INPUT_NDJSON_UNPARTITIONED",
   .input_data = []() { return std::vector<nlohmann::json>{}; },
   .database_config =
      R"(
schema:
  instanceName: "Test"
  metadata:
    - name: "accessionVersion"
      type: "string"
    - name: "2"
      type: "string"
  primaryKey: "accessionVersion"
)",
   .reference_genomes = R"(
{
  "nucleotideSequences": [
    {
      "name": "main",
      "sequence": "A"
    },
    {
      "name": "3",
      "sequence": "AA"
    }
  ],
  "genes": [
    {
      "name": "someGene",
      "sequence": "AA*"
    },
    {
      "name": "4",
      "sequence": "A*"
    }
  ]
})",
   .assertion{
      .expected_sequence_count = 0,
      .query = R"(
      {
         "action": {
           "type": "Details"
         },
         "filterExpression": {
            "type": "True"
         }
      }
   )",
      .expected_query_result = nlohmann::json::parse(R"(
[])")
   }
};

const Scenario<Success> NO_GENES = {
   .test_name = "NO_GENES",
   .input_data =
      []() {
         std::vector<nlohmann::json> result;
         for (size_t i = 0; i < 100; i++) {
            result.push_back(nlohmann::json::parse(fmt::format(
               R"({{
"accessionVersion": "{}.1",
"main": null,
"unaligned_main": null
}})",
               i
            )));
         }
         return result;
      },
   .database_config =
      R"(
schema:
  instanceName: "Test"
  metadata:
    - name: "accessionVersion"
      type: "string"
  primaryKey: "accessionVersion"
)",
   .reference_genomes = R"(
{
  "nucleotideSequences": [
    {
      "name": "main",
      "sequence": "ACGT"
    }
  ],
  "genes": []
})",
   .assertion{
      .expected_sequence_count = 100,
      .query = R"(
      {
         "action": {
           "type": "Aggregated"
         },
         "filterExpression": {
            "type": "True"
         }
      }
   )",
      .expected_query_result = nlohmann::json::parse(R"(
[{"count":100}])")
   }
};

const Scenario<Success> NO_NUCLEOTIDE_SEQUENCES = {
   .test_name = "NO_NUCLEOTIDE_SEQUENCES",
   .input_data =
      []() {
         std::vector<nlohmann::json> result;
         for (size_t i = 0; i < 100; i++) {
            result.push_back(nlohmann::json::parse(fmt::format(
               R"({{
"accessionVersion": "{}.1",
"someGene": {{"sequence": "AAAA", "insertions": []}}
}})",
               i
            )));
         }
         return result;
      },
   .database_config =
      R"(
schema:
  instanceName: "Test"
  metadata:
    - name: "accessionVersion"
      type: "string"
  primaryKey: "accessionVersion"
)",
   .reference_genomes = R"(
{
  "nucleotideSequences": [],
  "genes": [
    {
      "name": "someGene",
      "sequence": "AAC*"
    }
  ]
})",
   .assertion{
      .expected_sequence_count = 100,
      .query = R"(
      {
         "action": {
           "type": "Aggregated"
         },
         "filterExpression": {
            "type": "True"
         }
      }
   )",
      .expected_query_result = nlohmann::json::parse(R"(
[{"count":100}])")
   }
};

const Scenario<Success> NO_SEQUENCES = {
   .test_name = "NO_SEQUENCES",
   .input_data =
      []() {
         std::vector<nlohmann::json> result;
         for (size_t i = 0; i < 100; i++) {
            result.push_back(nlohmann::json::parse(fmt::format(
               R"({{
"accessionVersion": "{}.1"
}})",
               i
            )));
         }
         return result;
      },
   .database_config =
      R"(
schema:
  instanceName: "Test"
  metadata:
    - name: "accessionVersion"
      type: "string"
  primaryKey: "accessionVersion"
)",
   .reference_genomes = R"(
{
  "nucleotideSequences": [],
  "genes": []
}
)",
   .assertion{
      .expected_sequence_count = 100,
      .query = R"(
      {
         "action": {
           "type": "Aggregated"
         },
         "filterExpression": {
            "type": "True"
         }
      }
   )",
      .expected_query_result = nlohmann::json::parse(R"(
[{"count":100}])")
   }
};

const Scenario<Success> DIVERSE_SEQUENCE_NAMES_NDJSON = {
   .test_name = "DIVERSE_SEQUENCE_NAMES_NDJSON",
   .input_data =
      []() {
         std::vector<nlohmann::json> result;
         result.push_back(nlohmann::json::parse(R"({
"accessionVersion":"1.1",
"\"":{"sequence":"ACGTACGT","insertions":[]},
".":{"sequence":"ACGT","insertions":[]},
"---":{"sequence":"MYSF","insertions":[]},
"\"\\\"":{"sequence":"MADS","insertions":[]},
";|\\$!":{"sequence":"MSDN","insertions":[]},
"S-;":{"sequence":"MESL","insertions":[]},
"≤":{"sequence":"RVC*","insertions":[]},
"ł":{"sequence":"MDL*","insertions":[]},
"select":{"sequence":"MFH*","insertions":[]},
"S_-%":{"sequence":"MKI*","insertions":[]},
"S:;":{"sequence":"MIELSLIDFYLCFLAFLLFLVLIMLIIFWFSLELQDHNETCHA*","insertions":[]},
"#":{"sequence":"MKF*","insertions":[]},
"{{}}":{"sequence":"MDP*","insertions":[]},
"•":{"sequence":"MFV*","insertions":[]},
"unaligned_\"":"ACGTACGT",
"unaligned_.":"ACGT"
})"));
         result.push_back(nlohmann::json::parse(R"({
"accessionVersion":"1.2",
"\"":{"sequence":"ACGTACGT","insertions":[]},
".":{"sequence":"ACGT","insertions":[]},
"---":{"sequence":"MYSF","insertions":[]},
"\"\\\"":{"sequence":"MADS","insertions":[]},
";|\\$!":{"sequence":"MSDN","insertions":[]},
"S-;":{"sequence":"MESL","insertions":[]},
"≤":{"sequence":"RVC*","insertions":[]},
"ł":{"sequence":"MDL*","insertions":[]},
"select":{"sequence":"MFH*","insertions":[]},
"S_-%":{"sequence":"MKI*","insertions":[]},
"S:;":{"sequence":"MIELSLIDFYLCFLAFLLFLVLIMLIIFWFSLELQDHNETCHA*","insertions":[]},
"#":{"sequence":"MKF*","insertions":[]},
"{{}}":{"sequence":"MDP*","insertions":[]},
"•":{"sequence":"MFV*","insertions":[]},
"unaligned_\"":"ACGTACGT",
"unaligned_.":"ACGT"
})"));
         return result;
      },
   .database_config =
      R"(
schema:
  instanceName: "Test"
  metadata:
    - name: "accessionVersion"
      type: "string"
  primaryKey: "accessionVersion"
)",
   .reference_genomes = R"(
{
  "nucleotideSequences": [
    {
      "name": "\"",
      "sequence": "ACGTACGT"
    },
    {
      "name": ".",
      "sequence": "ACGT"
    }
  ],
  "genes": [
    {
      "name": "---",
      "sequence": "MYSF"
    },
    {
      "name": "\"\\\"",
      "sequence": "MADS"
    },
    {
      "name": ";|\\$!",
      "sequence": "MSDN"
    },
    {
      "name": "S-;",
      "sequence": "MESL"
    },
    {
      "name": "≤",
      "sequence": "RVC*"
    },
    {
      "name": "ł",
      "sequence": "MDL*"
    },
    {
      "name": "select",
      "sequence": "MFH*"
    },
    {
      "name": "S_-%",
      "sequence": "MKI*"
    },
    {
      "name": "S:;",
      "sequence": "MIELSLIDFYLCFLAFLLFLVLIMLIIFWFSLELQDHNETCHA*"
    },
    {
      "name": "#",
      "sequence": "MKF*"
    },
    {
      "name": "{{}}",
      "sequence": "MDP*"
    },
    {
      "name": "•",
      "sequence": "MFV*"
    }
  ]
})",
   .assertion{
      .expected_sequence_count = 2,
      .query = R"(
      {
         "action": {
           "type": "Aggregated"
         },
         "filterExpression": {
            "type": "True"
         }
      }
   )",
      .expected_query_result = nlohmann::json::parse(R"(
[{"count":2}])")
   }
};

const Scenario<Success> PREVENT_LATE_AUTO_CASTING = {
   .test_name = "PREVENT_LATE_AUTO_CASTING",
   .input_data =
      []() {
         std::vector<nlohmann::json> result;
         result.push_back(nlohmann::json::parse(R"({
"accessionVersion": "0"
})"));
         result.push_back(nlohmann::json::parse(R"({
"accessionVersion": "0.12"
})"));
         result.push_back(nlohmann::json::parse(R"({
"accessionVersion": "text_without_quotes"
})"));
         return result;
      },
   .database_config =
      R"(
schema:
  instanceName: "Test"
  metadata:
    - name: "accessionVersion"
      type: "string"
  primaryKey: "accessionVersion"
)",
   .reference_genomes = R"(
{
  "nucleotideSequences": [],
  "genes": []
})",
   .assertion{
      .expected_sequence_count = 3,
      .query = R"(
      {
         "action": {
           "type": "Details",
            "orderByFields": ["accessionVersion"]
         },
         "filterExpression": {
            "type": "True"
         }
      }
   )",
      .expected_query_result = nlohmann::json::parse(R"(
[{"accessionVersion":"0"},{"accessionVersion":"0.12"},{"accessionVersion":"text_without_quotes"}])")
   }
};

const Scenario<Success> TWO_LINEAGE_SYSTEMS = {
   .test_name = "TWO_LINEAGE_SYSTEMS",
   .input_data =
      []() {
         std::vector<nlohmann::json> result;
         result.push_back(nlohmann::json::parse(R"({
"accessionVersion": "0", "lineage_1": "root_1", "lineage_2": "root_2"
})"));
         result.push_back(nlohmann::json::parse(R"({
"accessionVersion": "1", "lineage_1": "child_1", "lineage_2": null
})"));
         result.push_back(nlohmann::json::parse(R"({
"accessionVersion": "2", "lineage_1": null, "lineage_2": "child_2"
})"));
         return result;
      },
   .database_config =
      R"(
schema:
  instanceName: "Test"
  metadata:
    - name: "accessionVersion"
      type: "string"
    - name: "lineage_1"
      type: "string"
      generateIndex: true
      generateLineageIndex: lineage_definition_1
    - name: "lineage_2"
      type: "string"
      generateIndex: true
      generateLineageIndex: lineage_definition_2
  primaryKey: "accessionVersion"
)",
   .reference_genomes = R"(
{
  "nucleotideSequences": [],
  "genes": []
})",
   .lineage_trees =
      {{"lineage_definition_1.yaml", R"(
root_1: ~
child_1:
  parents:
    - root_1
  )"},
       {"lineage_definition_2.yaml", R"(
root_2: ~
child_2:
  parents:
    - root_2)"}},
   .assertion{
      .expected_sequence_count = 3,
      .query = R"(
      {
         "action": {
           "type": "Details",
           "orderByFields": ["accessionVersion"]
         },
         "filterExpression": {
            "type": "Lineage",
            "column": "lineage_1",
            "value": "root_1",
            "includeSublineages": true
         }
      }
   )",
      .expected_query_result = nlohmann::json::parse(R"([
{"accessionVersion":"0","lineage_1":"root_1","lineage_2":"root_2"},
{"accessionVersion":"1","lineage_1":"child_1","lineage_2":null}
])")
   }
};

class PreprocessorTestFixture : public ::testing::TestWithParam<Scenario<Success>> {};

const auto testCases = ::testing::Values(
   NDJSON_FILE_WITH_MISSING_SEGMENTS_AND_GENES,
   NDJSON_WITH_SQL_KEYWORD_AS_FIELD,
   NDJSON_WITH_NUMERIC_NAMES,
   EMPTY_INPUT_NDJSON,
   EMPTY_INPUT_NDJSON_UNPARTITIONED,
   NO_GENES,
   NO_NUCLEOTIDE_SEQUENCES,
   NO_SEQUENCES,
   DIVERSE_SEQUENCE_NAMES_NDJSON,
   PREVENT_LATE_AUTO_CASTING,
   TWO_LINEAGE_SYSTEMS
);

INSTANTIATE_TEST_SUITE_P(PreprocessorTest, PreprocessorTestFixture, testCases, printTestName<Success>);

TEST_P(PreprocessorTestFixture, shouldProcessData) {
   const auto scenario = GetParam();

   auto preprocessing_config = prepareInputDirAndPreprocessorForScenario(scenario);

   auto database =
      std::make_shared<silo::Database>(silo::preprocessing::preprocessing(preprocessing_config));

   const auto database_info = database->getDatabaseInfo();

   EXPECT_EQ(database_info.sequence_count, scenario.assertion.expected_sequence_count);

   auto query = silo::query_engine::Query::parseQuery(scenario.assertion.query);
   auto query_plan = query->toQueryPlan(
      database, silo::config::RuntimeConfig::withDefaults().query_options, "some_id"
   );
   std::stringstream actual_result_stream;
   query_plan.executeAndWrite(&actual_result_stream, /*timeout_in_seconds=*/3);
   nlohmann::json actual_ndjson_result_as_array = nlohmann::json::array();
   std::string line;
   while (std::getline(actual_result_stream, line)) {
      auto line_object = nlohmann::json::parse(line);
      std::cout << line_object.dump() << std::endl;
      actual_ndjson_result_as_array.push_back(line_object);
   }

   ASSERT_EQ(actual_ndjson_result_as_array, scenario.assertion.expected_query_result);

   std::filesystem::remove_all(preprocessing_config.initialization_files.directory);
}

class InvalidPreprocessorTestFixture : public ::testing::TestWithParam<Scenario<Error>> {};

const Scenario<Error> DUPLICATE_PRIMARY_KEY{
   .test_name = "duplicate_primary_key",
   .input_data =
      []() {
         std::vector<nlohmann::json> result;
         result.emplace_back(nlohmann::json::parse(R"({"primaryKey": "id_1"})"));
         result.emplace_back(nlohmann::json::parse(R"({"primaryKey": "id_1"})"));
         result.emplace_back(nlohmann::json::parse(R"({"primaryKey": "id_2"})"));
         result.emplace_back(nlohmann::json::parse(R"({"primaryKey": "id_2"})"));
         return result;
      },
   .database_config =
      R"(
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
  primaryKey: "primaryKey"
)",
   .reference_genomes = R"(
{
  "nucleotideSequences": [],
  "genes": []
})",
   .assertion = {.error_message = "Found duplicate primary key id_1"}
};

const Scenario<Error> MISSING_NUCLEOTIDE_SEQUENCE_INPUT = {
   .test_name = "MISSING_NUCLEOTIDE_SEQUENCE_INPUT",
   .input_data =
      []() {
         std::vector<nlohmann::json> result;
         for (size_t i = 0; i < 100; i++) {
            result.emplace_back(
               nlohmann::json::parse(fmt::format(R"({{"accessionVersion": "{}.1"}})", i))
            );
         }
         return result;
      },
   .database_config =
      R"(
schema:
  instanceName: "Test"
  metadata:
    - name: "accessionVersion"
      type: "string"
  primaryKey: "accessionVersion"
)",
   .reference_genomes = R"(
{
  "nucleotideSequences": [
    {
      "name": "someSequence",
      "sequence": "A"
    }
],
  "genes": []
}
)",
   .assertion{
      .error_message =
         R"(preprocessing - exception when appending data: the column 'someSequence' is not contained in the object - current line: {"accessionVersion":"0.1"})"
   }
};

const Scenario<Error> TYPE_ERROR = {
   .test_name = "TYPE_ERROR",
   .input_data =
      []() {
         std::vector<nlohmann::json> result{nlohmann::json::parse(R"({
"accessionVersion": 0
})")};
         return result;
      },
   .database_config =
      R"(
schema:
  instanceName: "Test"
  metadata:
    - name: "accessionVersion"
      type: "string"
  primaryKey: "accessionVersion"
)",
   .reference_genomes = R"(
{
  "nucleotideSequences": [],
  "genes": []
}
)",
   .assertion{
      .error_message =
         R"(preprocessing - exception when appending data: When trying to get string value of column 'accessionVersion' got error: INCORRECT_TYPE: The JSON element does not have the requested type. - current line: {"accessionVersion":0})"
   }
};

const Scenario<Error> SEQUENCE_ILLEGAL_SYMBOL = {
   .test_name = "SEQUENCE_ILLEGAL_SYMBOL",
   .input_data =
      []() {
         std::vector<nlohmann::json> result{nlohmann::json::parse(R"({
"accessionVersion": "1.3",
"main": {"sequence": "ACET", "insertions": []},
"unaligned_main": "ACGT"
})")};
         return result;
      },
   .database_config =
      R"(
schema:
  instanceName: "Test"
  metadata:
    - name: "accessionVersion"
      type: "string"
  primaryKey: "accessionVersion"
)",
   .reference_genomes = R"(
{
  "nucleotideSequences": [
    {
      "name": "main",
      "sequence": "ACGT"
    }
  ],
  "genes": []
})",
   .assertion{
      .error_message =
         R"(Illegal character 'E' at position 2 contained in sequence with index 0 in the current buffer.)"
   }
};

const Scenario<Error> NDJSON_FILE_WITH_SOME_MISSING_KEYS = {
   .test_name = "NDJSON_FILE_WITH_SOME_MISSING_KEYS",
   .input_data =
      []() {
         std::vector<nlohmann::json> result;
         result.push_back(nlohmann::json::parse(R"({
"accessionVersion": "1.1",
"country": "Switzerland",
"main": {"sequence": "ACET", "insertions": ["123:RNRNRN"]},
"secondSegment": {"sequence": "ACET", "insertions": ["123:RNRNRN"]},
"someLongGene": {"sequence": "ACDEFGHIKLMNPQRSTVWYBZX", "insertions": []},
"someShortGene": {"sequence": "MADS", "insertions": ["123:RN"]},
"unaligned_main": "ACGT",
"unaligned_secondSegment": null
})"));
         result.push_back(nlohmann::json::parse(R"({
"accessionVersion": "1.3"
})"));
         return result;
      },
   .database_config =
      R"(
schema:
  instanceName: "Test"
  metadata:
    - name: "accessionVersion"
      type: "string"
    - name: "country"
      type: "string"
  primaryKey: "accessionVersion"
)",
   .reference_genomes = R"(
{
  "nucleotideSequences": [
    {
      "name": "main",
      "sequence": "ATTAAAGGTTTATACCTTCCCAGGTAACAAACCAACCAACTTTCGATCT"
    },
    {
      "name": "secondSegment",
      "sequence": "AAAAAAAAAAAAAAAA"
    }
  ],
  "genes": [
    {
      "name": "someLongGene",
      "sequence": "AAAAAAAAAAAAAAAAAAAAAAAAA"
    },
    {
      "name": "someShortGene",
      "sequence": "MADS"
    }
  ]
})",
   .assertion{
      .error_message =
         R"(preprocessing - exception when appending data: Did not find the field 'country' in the given json - current line: {"accessionVersion":"1.3"})"
   }
};

INSTANTIATE_TEST_SUITE_P(PreprocessorTest, InvalidPreprocessorTestFixture, ::testing::Values(DUPLICATE_PRIMARY_KEY, MISSING_NUCLEOTIDE_SEQUENCE_INPUT, TYPE_ERROR, SEQUENCE_ILLEGAL_SYMBOL, NDJSON_FILE_WITH_SOME_MISSING_KEYS), printTestName<Error>);

TEST_P(InvalidPreprocessorTestFixture, shouldNotProcessData) {
   const auto scenario = GetParam();

   auto preprocessing_config = prepareInputDirAndPreprocessorForScenario(scenario);
   EXPECT_THAT(
      // NOLINTNEXTLINE(clang-diagnostic-error)
      [&]() { silo::preprocessing::preprocessing(preprocessing_config); },
      ThrowsMessage<PreprocessingException>(::testing::HasSubstr(scenario.assertion.error_message))
   );
   std::filesystem::remove_all(preprocessing_config.initialization_files.directory);
}

}  // namespace
