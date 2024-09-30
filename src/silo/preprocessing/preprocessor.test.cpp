#include "silo/preprocessing/preprocessor.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "silo/config/util/config_repository.h"
#include "silo/config/util/yaml_file.h"
#include "silo/database.h"
#include "silo/database_info.h"
#include "silo/preprocessing/sql_function.h"
#include "silo/query_engine/query_engine.h"

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

// First 10 characters of sequence in nuc_main.sam have been removed
// This is to test the offset (set to 10 for both reads)
const Scenario SAM_FILES = {
   .input_directory = "testBaseData/samFiles/",
   .expected_sequence_count = 2,
   .query = R"(
      {
         "action": {
           "type": "FastaAligned",
           "sequenceName": ["main"],
           "orderByFields": ["accessionVersion"]
         },
         "filterExpression": {
            "type": "True"
         }
      }
   )",
   .expected_query_result = nlohmann::json::parse(R"([
         {
            "accessionVersion": "1.1",
            "main": "NNNNNNNNNNTATACCTTCCCAGGTAACAAACCAACCAACTTTCGATCT"
         },
         {
            "accessionVersion": "1.3",
            "main": "NNNNNTATACCTTCCCAGGTAACAAACCAACCAACTTTCGATCTNNNNN"
         }
      ])")
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

const Scenario TSV_FILE_WITH_SQL_KEYWORD_AS_FIELD = {
   .input_directory = "testBaseData/tsvWithSqlKeywordField/",
   .expected_sequence_count = NDJSON_WITH_SQL_KEYWORD_AS_FIELD.expected_sequence_count,
   .query = NDJSON_WITH_SQL_KEYWORD_AS_FIELD.query,
   .expected_query_result = NDJSON_WITH_SQL_KEYWORD_AS_FIELD.expected_query_result
};

const Scenario TSV_FILE_WITH_QUOTE_IN_FIELD_NAME = {
   .input_directory = "testBaseData/tsvWithQuoteInFieldName/",
   .expected_sequence_count = 2,
   .query = R"(
{
   "action": {
      "type": "Aggregated",
      "groupByFields": ["x\"y"],
      "orderByFields": ["x\"y"]
   },
   "filterExpression": {
      "type": "StringEquals",
      "column": "x\"y",
      "value": "a"
   }
}
   )",
   .expected_query_result = nlohmann::json::parse(
      R"([
         {"count": 1, "x\"y": "a"}
   ])"
   )
};

const Scenario TSV_FILE_WITH_QUOTE_IN_PARTITION_BY = {
   .input_directory = "testBaseData/tsvWithQuoteInPartitionBy/",
   .expected_sequence_count = 100,
   .query = R"(
{
   "action": {
      "type": "Aggregated",
      "groupByFields": ["pango_\"lineage"],
      "orderByFields": ["pango_\"lineage"],
      "limit": 3
   },
   "filterExpression": {
      "type": "True"
   }
}
   )",
   .expected_query_result = nlohmann::json::parse(
      R"([
         {"count":1,"pango_\"lineage":null},
         {"count":1,"pango_\"lineage":"AY.122"},
         {"count":4,"pango_\"lineage":"AY.43"}
   ])"
   )
};

const Scenario EMPTY_INPUT_TSV = {
   .input_directory = "testBaseData/emptyInputTsv/",
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

const Scenario DIVERSE_SEQUENCE_NAMES = {
   .input_directory = "testBaseData/diverseSequenceNames/",
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

class PreprocessorTestFixture : public ::testing::TestWithParam<Scenario> {};

INSTANTIATE_TEST_SUITE_P(
   PreprocessorTest,
   PreprocessorTestFixture,
   ::testing::Values(
      DIVERSE_SEQUENCE_NAMES,
      DIVERSE_SEQUENCE_NAMES_NDJSON,
      FASTA_FILES_WITH_MISSING_SEGMENTS_AND_GENES,
      NDJSON_FILE_WITH_MISSING_SEGMENTS_AND_GENES,
      SAM_FILES,
      NDJSON_WITH_SQL_KEYWORD_AS_FIELD,
      TSV_FILE_WITH_SQL_KEYWORD_AS_FIELD,
      TSV_FILE_WITH_QUOTE_IN_FIELD_NAME,
      TSV_FILE_WITH_QUOTE_IN_PARTITION_BY,
      NDJSON_WITH_NUMERIC_NAMES,
      EMPTY_INPUT_TSV,
      EMPTY_INPUT_NDJSON,
      EMPTY_INPUT_NDJSON_UNPARTITIONED,
      NO_GENES,
      NO_NUCLEOTIDE_SEQUENCES,
      NO_SEQUENCES
   ),
   printTestName
);

TEST_P(PreprocessorTestFixture, shouldProcessData) {
   const auto scenario = GetParam();
   silo::config::PreprocessingConfig config{.input_directory = scenario.input_directory};

   config.overwrite(silo::config::YamlFile(scenario.input_directory / "preprocessing_config.yaml"));

   const auto database_config = silo::config::ConfigRepository().getValidatedConfig(
      scenario.input_directory / "database_config.yaml"
   );

   const auto reference_genomes =
      silo::ReferenceGenomes::readFromFile(config.getReferenceGenomeFilename());

   const auto alias_lookup =
      silo::PangoLineageAliasLookup::readFromFile(config.getPangoLineageDefinitionFilename());
   silo::preprocessing::Preprocessor preprocessor(
      config, database_config, reference_genomes, alias_lookup
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
