#include "silo/preprocessing/preprocessing.h"

#include <fstream>
#include <functional>
#include <vector>

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

struct NdjsonInputLine {
   std::map<std::string, nlohmann::json> metadata;
   std::map<std::string, nlohmann::json> alignedNucleotideSequences;
   std::map<std::string, nlohmann::json> unalignedNucleotideSequences;
   std::map<std::string, nlohmann::json> alignedAminoAcidSequences;
   std::map<std::string, nlohmann::json> nucleotideInsertions;
   std::map<std::string, nlohmann::json> aminoAcidInsertions;

   [[nodiscard]] nlohmann::json toJson() const {
      return nlohmann::json{
         {"metadata", metadata},
         {"alignedNucleotideSequences", alignedNucleotideSequences},
         {"unalignedNucleotideSequences", unalignedNucleotideSequences},
         {"alignedAminoAcidSequences", alignedAminoAcidSequences},
         {"nucleotideInsertions", nucleotideInsertions},
         {"aminoAcidInsertions", aminoAcidInsertions}
      };
   }
};

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
   std::function<std::vector<NdjsonInputLine>()> input_data;
   std::string database_config;
   std::string reference_genomes;
   std::string lineage_tree;
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

   std::ofstream lineage_definitions_file(input_directory / "lineage_definitions.yaml");
   lineage_definitions_file << scenario.lineage_tree;
   lineage_definitions_file.close();

   auto config_with_input_dir = PreprocessingConfig::withDefaults();
   config_with_input_dir.initialization_files.directory = input_directory;
   config_with_input_dir.input_file = "input.json";
   config_with_input_dir.validate();

   std::ofstream file(config_with_input_dir.getInputFilePath().value());

   if (!file.is_open()) {
      throw std::runtime_error("Could not open file for writing ndjson data");
   }
   for (const auto& line : scenario.input_data()) {
      file << line.toJson().dump() << std::endl;
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
         std::vector<NdjsonInputLine> result;
         result.push_back(
            {.metadata = {{"accessionVersion", "1.1"}, {"country", "Switzerland"}},
             .alignedNucleotideSequences = {{"main", "NNACTGNN"}, {"secondSegment", nullptr}},
             .unalignedNucleotideSequences =
                {{"main", "ATTAAAGGTTTATACCTTCCCAGGTAACAAACCAACCAACTTTCGATCT"},
                 {"secondSegment", nullptr}},
             .alignedAminoAcidSequences{
                {"someLongGene", "ACDEFGHIKLMNPQRSTVWYBZX-*"}, {"someShortGene", "MADS"}
             },
             .nucleotideInsertions = {{"main", {"123:ACTG"}}, {"secondSegment", {}}},
             .aminoAcidInsertions =
                {{"someLongGene", {"123:RNRNRN"}}, {"someShortGene", {"123:RN"}}}}
         );
         result.push_back(
            {.metadata = {{"accessionVersion", "1.3"}, {"country", "Germany"}},
             .alignedNucleotideSequences = {{"main", nullptr}, {"secondSegment", nullptr}},
             .unalignedNucleotideSequences = {{"main", nullptr}, {"secondSegment", nullptr}},
             .alignedAminoAcidSequences{{"someLongGene", nullptr}, {"someShortGene", nullptr}},
             .nucleotideInsertions = {{"main", {}}, {"secondSegment", {}}},
             .aminoAcidInsertions = {{"someLongGene", {}}, {"someShortGene", {}}}}
         );
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
         std::vector<NdjsonInputLine> result;
         result.push_back(
            {.metadata = {{"primaryKey", "1.1"}, {"group", "dummyValue"}},
             .alignedNucleotideSequences = {{"main", "A"}},
             .unalignedNucleotideSequences = {{"main", "A"}},
             .alignedAminoAcidSequences{{"mainGene", "A*"}},
             .nucleotideInsertions = {{"main", {}}},
             .aminoAcidInsertions = {{"mainGene", {}}}}
         );
         result.push_back(
            {.metadata = {{"primaryKey", "1.2"}, {"group", nullptr}},
             .alignedNucleotideSequences = {{"main", nullptr}},
             .unalignedNucleotideSequences = {{"main", nullptr}},
             .alignedAminoAcidSequences{{"mainGene", nullptr}},
             .nucleotideInsertions = {{"main", {}}},
             .aminoAcidInsertions = {{"mainGene", {}}}}
         );
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
         std::vector<NdjsonInputLine> result;
         result.push_back(
            {.metadata = {{"2", "google.com"}, {"accessionVersion", "1.1"}},
             .alignedNucleotideSequences = {{"main", "A"}, {"3", "AA"}},
             .unalignedNucleotideSequences = {{"main", "A"}, {"3", "AA"}},
             .alignedAminoAcidSequences{{"someGene", "AA*"}, {"4", "A*"}},
             .nucleotideInsertions = {{"main", {}}, {"3", {}}},
             .aminoAcidInsertions = {{"someGene", {}}, {"4", {}}}}
         );
         result.push_back(
            {.metadata = {{"2", nullptr}, {"accessionVersion", "1.3"}},
             .alignedNucleotideSequences = {{"main", nullptr}, {"3", nullptr}},
             .unalignedNucleotideSequences = {{"main", nullptr}, {"3", nullptr}},
             .alignedAminoAcidSequences{{"someGene", nullptr}, {"4", nullptr}},
             .nucleotideInsertions = {{"main", {}}, {"3", {}}},
             .aminoAcidInsertions = {{"someGene", {}}, {"4", {}}}}
         );
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
   .input_data = []() { return std::vector<NdjsonInputLine>{}; },
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
      generateLineageIndex: true
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

const Scenario<Success> EMPTY_INPUT_NDJSON_UNPARTITIONED = {
   .test_name = "EMPTY_INPUT_NDJSON_UNPARTITIONED",
   .input_data = []() { return std::vector<NdjsonInputLine>{}; },
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
         std::vector<NdjsonInputLine> result;
         for (size_t i = 0; i < 100; i++) {
            result.push_back(
               {.metadata = {{"accessionVersion", fmt::format("{}.1", i)}},
                .alignedNucleotideSequences = {{"main", "ACGT"}},
                .unalignedNucleotideSequences = {{"main", "ACGT"}},
                .nucleotideInsertions = {{"main", {}}}}
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
         std::vector<NdjsonInputLine> result;
         for (size_t i = 0; i < 100; i++) {
            result.push_back(
               {.metadata = {{"accessionVersion", fmt::format("{}.1", i)}},
                .alignedAminoAcidSequences = {{"someGene", "AAAA"}},
                .aminoAcidInsertions = {{"someGene", {}}}}
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
         std::vector<NdjsonInputLine> result;
         for (size_t i = 0; i < 100; i++) {
            result.push_back({.metadata = {{"accessionVersion", fmt::format("{}.1", i)}}});
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
         std::vector<NdjsonInputLine> result;
         for (size_t i = 0; i < 2; i++) {
            result.push_back(
               {.metadata = {{"accessionVersion", fmt::format("{}.1", i)}},
                .alignedNucleotideSequences = {{"\"", "ACGTACGT"}, {".", "ACGT"}},
                .unalignedNucleotideSequences = {{"\"", "ACGTACGT"}, {".", "ACGT"}},
                .alignedAminoAcidSequences =
                   {{"---", "MYSF"},
                    {"\"\\\"", "MADS"},
                    {";|\\$!", "MSDN"},
                    {"S-;", "MESL"},
                    {"≤", "RVC*"},
                    {"ł", "MDL*"},
                    {"select", "MFH*"},
                    {"S_-%", "MKI*"},
                    {"S:;", "MIELSLIDFYLCFLAFLLFLVLIMLIIFWFSLELQDHNETCHA*"},
                    {"#", "MKF*"},
                    {"{{}}", "MDP*"},
                    {"•", "MFV*"}},
                .nucleotideInsertions = {{"\"", {}}, {".", {}}},
                .aminoAcidInsertions =
                   {{"---", {}},
                    {"\"\\\"", {}},
                    {";|\\$!", {}},
                    {"S-;", {}},
                    {"≤", {}},
                    {"ł", {}},
                    {"select", {}},
                    {"S_-%", {}},
                    {"S:;", {}},
                    {"#", {}},
                    {"{{}}", {}},
                    {"•", {}}}}
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
         std::vector<NdjsonInputLine> result;
         result.push_back({.metadata = {{"accessionVersion", "0"}}});
         result.push_back({.metadata = {{"accessionVersion", "0.12"}}});
         result.push_back({.metadata = {{"accessionVersion", "text_without_quotes"}}});
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
   PREVENT_LATE_AUTO_CASTING
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
   auto query_plan =
      query->toQueryPlan(database, silo::config::RuntimeConfig::withDefaults().query_options);
   std::stringstream actual_result_stream;
   query_plan.executeAndWrite(&actual_result_stream);
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
         std::vector<NdjsonInputLine> result;
         result.emplace_back(NdjsonInputLine{.metadata = {{"primaryKey", "id_1"}}});
         result.emplace_back(NdjsonInputLine{.metadata = {{"primaryKey", "id_1"}}});
         result.emplace_back(NdjsonInputLine{.metadata = {{"primaryKey", "id_2"}}});
         result.emplace_back(NdjsonInputLine{.metadata = {{"primaryKey", "id_2"}}});
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
         std::vector<NdjsonInputLine> result;
         for (size_t i = 0; i < 100; i++) {
            result.push_back({.metadata = {{"accessionVersion", fmt::format("{}.1", i)}}});
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
         R"(preprocessing - exception when appending data: The following line does not conform to SILO's json specification when adding to database column someSequence: '{"alignedAminoAcidSequences":{},"alignedNucleotideSequences":{},"aminoAcidInsertions":{},"metadata":{"accessionVersion":"0.1"},"nucleotideInsertions":{},"unalignedNucleotideSequences":{}}'
We got a json out_of_range error, indicating that an expected field was not present: [json.exception.out_of_range.403] key 'someSequence' not found)"
   }
};

const Scenario<Error> TYPE_ERROR = {
   .test_name = "TYPE_ERROR",
   .input_data =
      []() {
         std::vector<NdjsonInputLine> result{{.metadata = {{"accessionVersion", 0}}}};
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
         R"(We got a json type_error: [json.exception.type_error.302] type must be string, but is number)"
   }
};

const Scenario<Error> SEQUENCE_ILLEGAL_SYMBOL = {
   .test_name = "SEQUENCE_ILLEGAL_SYMBOL",
   .input_data =
      []() {
         std::vector<NdjsonInputLine> result{
            {.metadata = {{"accessionVersion", "1"}},
             .alignedNucleotideSequences = {{"main", "ACET"}},
             .unalignedNucleotideSequences = {{"main", "ACGT"}},
             .nucleotideInsertions = {{"main", {}}}}
         };
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
         std::vector<NdjsonInputLine> result;
         result.push_back(
            {.metadata = {{"accessionVersion", "1.1"}, {"country", "Switzerland"}},
             .alignedNucleotideSequences = {{"main", "NNACTGNN"}, {"secondSegment", nullptr}},
             .unalignedNucleotideSequences =
                {{"main", "ATTAAAGGTTTATACCTTCCCAGGTAACAAACCAACCAACTTTCGATCT"},
                 {"secondSegment", nullptr}},
             .alignedAminoAcidSequences{
                {"someLongGene", "ACDEFGHIKLMNPQRSTVWYBZX-*"}, {"someShortGene", "MADS"}
             },
             .nucleotideInsertions = {{"main", {"123:ACTG"}}, {"secondSegment", {}}},
             .aminoAcidInsertions =
                {{"someLongGene", {"123:RNRNRN"}}, {"someShortGene", {"123:RN"}}}}
         );
         result.push_back(
            {.metadata = {{"accessionVersion", "1.3"}},
             .alignedNucleotideSequences = {{"main", nullptr}, {"secondSegment", nullptr}},
             .unalignedNucleotideSequences = {{"main", nullptr}, {"secondSegment", nullptr}},
             .alignedAminoAcidSequences{{"someLongGene", nullptr}, {"someShortGene", nullptr}},
             .nucleotideInsertions = {{"main", {}}, {"secondSegment", {}}},
             .aminoAcidInsertions = {{"someLongGene", {}}, {"someShortGene", {}}}}
         );
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
         R"(preprocessing - exception when appending data: The following line does not conform to SILO's json specification when adding to database column country: '{"alignedAminoAcidSequences":{"someLongGene":null,"someShortGene":null},"alignedNucleotideSequences":{"main":null,"secondSegment":null},"aminoAcidInsertions":{"someLongGene":null,"someShortGene":null},"metadata":{"accessionVersion":"1.3"},"nucleotideInsertions":{"main":null,"secondSegment":null},"unalignedNucleotideSequences":{"main":null,"secondSegment":null}}'
We got a json out_of_range error, indicating that an expected field was not present: [json.exception.out_of_range.403] key 'country' not found)"
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
