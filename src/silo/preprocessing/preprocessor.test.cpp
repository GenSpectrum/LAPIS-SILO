#include "silo/preprocessing/preprocessor.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "config/source/yaml_file.h"
#include "silo/config/util/config_repository.h"
#include "silo/database.h"
#include "silo/database_info.h"
#include "silo/preprocessing/sql_function.h"
#include "silo/query_engine/query_engine.h"

using silo::config::PreprocessingConfig;

namespace {

struct Scenario {
   std::filesystem::path input_directory;
   uint expected_sequence_count;
   std::string query;
   nlohmann::json expected_query_result;
};

std::string printTestName(const ::testing::TestParamInfo<Scenario>& info) {
   std::string name = "Dir_" + info.param.input_directory.string();
   std::ranges::replace(name, '/', '_');
   return name;
}

const Scenario NDJSON_FILE_WITH_MISSING_SEGMENTS_AND_GENES = {
   .input_directory = "testBaseData/ndjsonWithNullSequences/",
   .expected_sequence_count = 2,
   .query = R"(
      {
         "action": {
           "type": "FastaAligned",
           "sequenceName": ["someShortGene", "secondSegment"],
           "orderByFields": ["accessionVersion"]
         },
         "filterExpression": {
            "type": "True"
         }
      }
   )",
   .expected_query_result = nlohmann::json::parse(R"(
[{
   "accessionVersion": "1.1",
   "someShortGene": "MADS",
   "secondSegment": "NNNNNNNNNNNNNNNN"
},
{
   "accessionVersion": "1.3",
   "someShortGene": "XXXX",
   "secondSegment": "NNNNNNNNNNNNNNNN"
}])")
};

const Scenario NDJSON_WITH_SQL_KEYWORD_AS_FIELD = {
   .input_directory = "testBaseData/ndjsonWithSqlKeywordField/",
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
};

const Scenario NDJSON_WITH_NUMERIC_NAMES = {
   .input_directory = "testBaseData/numericNames/",
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
};

const Scenario EMPTY_INPUT_NDJSON = {
   .input_directory = "testBaseData/emptyInputNdjson/",
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
};

const Scenario EMPTY_INPUT_NDJSON_UNPARTITIONED = {
   .input_directory = "testBaseData/emptyInputNdjsonUnpartitioned/",
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
   .expected_query_result = nlohmann::json::parse(R"([])")
};

const Scenario NO_GENES = {
   .input_directory = "testBaseData/noGenes/",
   .expected_sequence_count = 9,
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
[{"count":9}])")
};

const Scenario NO_NUCLEOTIDE_SEQUENCES = {
   .input_directory = "testBaseData/noNucleotideSequences/",
   .expected_sequence_count = 30,
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
   .expected_query_result = nlohmann::json::parse(R"([{"count":30}])")
};

const Scenario NO_SEQUENCES = {
   .input_directory = "testBaseData/noSequences/",
   .expected_sequence_count = 6,
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
   .expected_query_result = nlohmann::json::parse(R"([{"count":6}])")
};

const Scenario DIVERSE_SEQUENCE_NAMES_NDJSON = {
   .input_directory = "testBaseData/diverseSequenceNamesAsNdjson/",
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
};

const Scenario PREVENT_LATE_AUTO_CASTING = {
   .input_directory = "testBaseData/autoCasting/",
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
};

class PreprocessorTestFixture : public ::testing::TestWithParam<Scenario> {};

INSTANTIATE_TEST_SUITE_P(
   PreprocessorTest,
   PreprocessorTestFixture,
   ::testing::Values(
      DIVERSE_SEQUENCE_NAMES_NDJSON,
      NDJSON_FILE_WITH_MISSING_SEGMENTS_AND_GENES,
      NDJSON_WITH_SQL_KEYWORD_AS_FIELD,
      NDJSON_WITH_NUMERIC_NAMES,
      EMPTY_INPUT_NDJSON,
      EMPTY_INPUT_NDJSON_UNPARTITIONED,
      NO_GENES,
      NO_NUCLEOTIDE_SEQUENCES,
      NO_SEQUENCES,
      PREVENT_LATE_AUTO_CASTING
   ),
   printTestName
);

TEST_P(PreprocessorTestFixture, shouldProcessData) {
   const auto scenario = GetParam();
   silo::config::PreprocessingConfig config;
   config.input_directory = scenario.input_directory;

   config.overwriteFrom(
      silo::config::YamlConfig::readFile(scenario.input_directory / "preprocessing_config.yaml")
         .verify(PreprocessingConfig::getConfigSpecification())
   );

   const auto database_config = silo::config::ConfigRepository().getValidatedConfig(
      scenario.input_directory / "database_config.yaml"
   );

   const auto reference_genomes =
      silo::ReferenceGenomes::readFromFile(config.getReferenceGenomeFilename());

   auto lineage_tree = silo::common::LineageTreeAndIdMap();
   if (config.getLineageDefinitionsFilename()) {
      lineage_tree = silo::common::LineageTreeAndIdMap::fromLineageDefinitionFilePath(
         config.getLineageDefinitionsFilename().value()
      );
   }
   silo::preprocessing::Preprocessor preprocessor(
      config, database_config, reference_genomes, std::move(lineage_tree)
   );
   auto database = preprocessor.preprocess();

   const auto database_info = database.getDatabaseInfo();

   EXPECT_EQ(database_info.sequence_count, scenario.expected_sequence_count);

   const silo::query_engine::QueryEngine query_engine(database);
   const auto result = query_engine.executeQuery(scenario.query);

   const auto actual = nlohmann::json(result.entries());
   ASSERT_EQ(actual, scenario.expected_query_result);
}

}  // namespace
