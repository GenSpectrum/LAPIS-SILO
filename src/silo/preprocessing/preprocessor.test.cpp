#include "silo/preprocessing/preprocessor.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "silo/config/config_repository.h"
#include "silo/database.h"
#include "silo/database_info.h"
#include "silo/preprocessing/preprocessing_config_reader.h"
#include "silo/preprocessing/sql_function.h"
#include "silo/query_engine/query_engine.h"

namespace {

struct Scenario {
   std::string input_directory;
   uint expected_sequence_count;
   std::string query;
   nlohmann::json expected_query_result;
};

std::string printTestName(const ::testing::TestParamInfo<Scenario>& info) {
   std::string name = "Dir_" + info.param.input_directory;
   std::replace(name.begin(), name.end(), '/', '_');
   return name;
}

const Scenario FASTA_FILES_WITH_MISSING_SEGMENTS_AND_GENES = {
   .input_directory = "testBaseData/fastaFilesWithMissingSequences/",
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

const Scenario NDJSON_FILE_WITH_MISSING_SEGMENTS_AND_GENES = {
   .input_directory = "testBaseData/ndjsonWithNullSequences/",
   .expected_sequence_count = FASTA_FILES_WITH_MISSING_SEGMENTS_AND_GENES.expected_sequence_count,
   .query = FASTA_FILES_WITH_MISSING_SEGMENTS_AND_GENES.query,
   .expected_query_result = FASTA_FILES_WITH_MISSING_SEGMENTS_AND_GENES.expected_query_result
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

const Scenario TSV_FILE_WITH_SQL_KEYWORD_AS_FIELD = {
   .input_directory = "testBaseData/tsvWithSqlKeywordField/",
   .expected_sequence_count = NDJSON_WITH_SQL_KEYWORD_AS_FIELD.expected_sequence_count,
   .query = NDJSON_WITH_SQL_KEYWORD_AS_FIELD.query,
   .expected_query_result = NDJSON_WITH_SQL_KEYWORD_AS_FIELD.expected_query_result
};

class PreprocessorTestFixture : public ::testing::TestWithParam<Scenario> {};

INSTANTIATE_TEST_SUITE_P(
   PreprocessorTest,
   PreprocessorTestFixture,
   ::testing::Values(
      FASTA_FILES_WITH_MISSING_SEGMENTS_AND_GENES,
      NDJSON_FILE_WITH_MISSING_SEGMENTS_AND_GENES,
      NDJSON_WITH_SQL_KEYWORD_AS_FIELD,
      TSV_FILE_WITH_SQL_KEYWORD_AS_FIELD
   ),
   printTestName
);

TEST_P(PreprocessorTestFixture, shouldProcessDataSetWithMissingSequences) {
   const auto scenario = GetParam();

   auto config = silo::preprocessing::PreprocessingConfigReader()
                    .readConfig(scenario.input_directory + "preprocessing_config.yaml")
                    .mergeValuesFromOrDefault(silo::preprocessing::OptionalPreprocessingConfig());

   const auto database_config = silo::config::ConfigRepository().getValidatedConfig(
      scenario.input_directory + "database_config.yaml"
   );

   const auto reference_genomes =
      silo::ReferenceGenomes::readFromFile(config.getReferenceGenomeFilename());

   silo::preprocessing::Preprocessor preprocessor(config, database_config, reference_genomes);
   auto database = preprocessor.preprocess();

   const auto database_info = database.getDatabaseInfo();

   EXPECT_GT(database_info.total_size, 0UL);
   EXPECT_EQ(database_info.sequence_count, scenario.expected_sequence_count);

   const silo::query_engine::QueryEngine query_engine(database);
   const auto result = query_engine.executeQuery(scenario.query);

   const auto actual = nlohmann::json(result.query_result);
   ASSERT_EQ(actual, scenario.expected_query_result);
}

}  // namespace
