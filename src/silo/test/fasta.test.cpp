#include <nlohmann/json.hpp>

#include <optional>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

namespace {
nlohmann::json createDataWithUnalignedSequences(
   const std::string& primaryKey,
   const std::string& date,
   const nlohmann::json& segment1,
   const nlohmann::json& segment2
) {
   return {
      {"primaryKey", primaryKey},
      {"date", date},
      {"segment1", nullptr},
      {"segment2", nullptr},
      {"unaligned_segment1", segment1},
      {"unaligned_segment2", segment2},
      {"gene1", nullptr},
      {"gene2", nullptr}
   };
}

const std::vector<nlohmann::json> DATA = {
   createDataWithUnalignedSequences("bothSegments", "2024-08-01", "A", "G"),
   createDataWithUnalignedSequences("onlySegment1", "2024-08-03", "T", nullptr),
   createDataWithUnalignedSequences("onlySegment2", "2024-08-02", nullptr, "T"),
   createDataWithUnalignedSequences("noSegment", "2024-08-08", nullptr, nullptr),
   createDataWithUnalignedSequences("1", "2024-08-05", nullptr, "A"),
   createDataWithUnalignedSequences("2", "2024-08-03", nullptr, nullptr),
   createDataWithUnalignedSequences("3", "2024-08-02", nullptr, "AA")
};

const auto DATABASE_CONFIG =
   R"(
 schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
    - name: "date"
      type: "date"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}, {"segment2", "T"}},
   {{"gene1", "*"}, {"gene2", "*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = DATA,
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

nlohmann::json createFastaAlignedQuery(const std::string& primaryKey) {
   return nlohmann::json::parse(fmt::format(
      R"(
{{
  "action": {{
    "type": "Fasta",
    "sequenceNames": [
      "unaligned_segment1",
      "unaligned_segment2"
    ]
  }},
  "filterExpression": {{
    "type": "StringEquals",
    "column": "primaryKey",
    "value": "{}"
  }}
}}
)",
      primaryKey
   ));
}

const QueryTestScenario SEQUENCE_WITH_BOTH_SEGMENTS_SCENARIO = {
   .name = "sequenceWithBothSegments",
   .query = createFastaAlignedQuery("bothSegments"),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "bothSegments"}, {"unaligned_segment1", "A"}, {"unaligned_segment2", "G"}}}
   )
};

const QueryTestScenario SEQUENCE_WITH_ONLY_FIRST_SEGMENT_SCENARIO = {
   .name = "sequenceWithOnlyFirstSegment",
   .query = createFastaAlignedQuery("onlySegment1"),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "onlySegment1"}, {"unaligned_segment1", "T"}, {"unaligned_segment2", nullptr}
      }}
   )
};

const QueryTestScenario SEQUENCE_WITH_ONLY_SECOND_SEGMENT_SCENARIO = {
   .name = "sequenceWithOnlySecondSegment",
   .query = createFastaAlignedQuery("onlySegment2"),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "onlySegment2"}, {"unaligned_segment1", nullptr}, {"unaligned_segment2", "T"}
      }}
   )
};

const QueryTestScenario SEQUENCE_WITH_NO_SEGMENT_SCENARIO = {
   .name = "sequenceWithNoSegment",
   .query = createFastaAlignedQuery("noSegment"),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "noSegment"},
        {"unaligned_segment1", nullptr},
        {"unaligned_segment2", nullptr}}}
   )
};

const QueryTestScenario DOWNLOAD_ALL_SEQUENCES_SCENARIO = {
   .name = "downloadAllSequences",
   .query = nlohmann::json::parse(R"(
{
  "action": {
    "type": "Fasta",
    "orderByFields": [
      "primaryKey"
    ],
    "sequenceNames": [
      "unaligned_segment1",
      "unaligned_segment2"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
}
)"),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "1"}, {"unaligned_segment1", nullptr}, {"unaligned_segment2", "A"}},
       {{"primaryKey", "2"}, {"unaligned_segment1", nullptr}, {"unaligned_segment2", nullptr}},
       {{"primaryKey", "3"}, {"unaligned_segment1", nullptr}, {"unaligned_segment2", "AA"}},
       {{"primaryKey", "bothSegments"}, {"unaligned_segment1", "A"}, {"unaligned_segment2", "G"}},
       {{"primaryKey", "noSegment"},
        {"unaligned_segment1", nullptr},
        {"unaligned_segment2", nullptr}},
       {{"primaryKey", "onlySegment1"}, {"unaligned_segment1", "T"}, {"unaligned_segment2", nullptr}
       },
       {{"primaryKey", "onlySegment2"}, {"unaligned_segment1", nullptr}, {"unaligned_segment2", "T"}
       }}
   ),
   .query_options = silo::config::QueryOptions{.materialization_cutoff = 0}
};

const QueryTestScenario DOWNLOAD_ALL_DATA = {
   .name = "DOWNLOAD_ALL_DATA",
   .query = nlohmann::json::parse(R"(
{
  "action": {
    "type": "Fasta",
    "orderByFields": [
      "primaryKey"
    ],
    "sequenceNames": [
      "unaligned_segment1",
      "unaligned_segment2"
    ],
    "additionalFields": [
      "date"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
}
)"),
   .expected_query_result = nlohmann::json::parse(R"(
[{"date":"2024-08-05","primaryKey":"1","unaligned_segment1":null,"unaligned_segment2":"A"},
{"date":"2024-08-03","primaryKey":"2","unaligned_segment1":null,"unaligned_segment2":null},
{"date":"2024-08-02","primaryKey":"3","unaligned_segment1":null,"unaligned_segment2":"AA"},
{"date":"2024-08-01","primaryKey":"bothSegments","unaligned_segment1":"A","unaligned_segment2":"G"},
{"date":"2024-08-08","primaryKey":"noSegment","unaligned_segment1":null,"unaligned_segment2":null},
{"date":"2024-08-03","primaryKey":"onlySegment1","unaligned_segment1":"T","unaligned_segment2":null},
{"date":"2024-08-02","primaryKey":"onlySegment2","unaligned_segment1":null,"unaligned_segment2":"T"}])"
   )
};

const QueryTestScenario DUPLICATE_FIELDS = {
   .name = "DUPLICATE_FIELDS",
   .query = nlohmann::json::parse(R"(
{
  "action": {
    "type": "Fasta",
    "orderByFields": [
      "primaryKey"
    ],
    "sequenceNames": [
      "unaligned_segment1",
      "unaligned_segment2",
      "unaligned_segment1"
    ],
    "additionalFields": [
      "date",
      "date"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
}
)"),
   .expected_query_result = nlohmann::json::parse(R"(
[{"date":"2024-08-05","primaryKey":"1","unaligned_segment1":null,"unaligned_segment2":"A"},
{"date":"2024-08-03","primaryKey":"2","unaligned_segment1":null,"unaligned_segment2":null},
{"date":"2024-08-02","primaryKey":"3","unaligned_segment1":null,"unaligned_segment2":"AA"},
{"date":"2024-08-01","primaryKey":"bothSegments","unaligned_segment1":"A","unaligned_segment2":"G"},
{"date":"2024-08-08","primaryKey":"noSegment","unaligned_segment1":null,"unaligned_segment2":null},
{"date":"2024-08-03","primaryKey":"onlySegment1","unaligned_segment1":"T","unaligned_segment2":null},
{"date":"2024-08-02","primaryKey":"onlySegment2","unaligned_segment1":null,"unaligned_segment2":"T"}])"
   )
};

const QueryTestScenario ORDER_BY_NOT_IN_OUTPUT = {
   .name = "ORDER_BY_NOT_IN_OUTPUT",
   .query = nlohmann::json::parse(R"(
{
  "action": {
    "sequenceNames": [
      "unaligned_segment1"
    ],
    "limit": 1,
    "orderByFields": [
      {
        "field": "date",
        "order": "descending"
      }
    ],
    "type": "Fasta"
  },
  "filterExpression": {
    "type": "True"
  }
}
)"),
   .expected_error_message =
      "OrderByField date is not contained in the result of this operation. "
      "The only fields returned by this action are primaryKey, unaligned_segment1"
};

const QueryTestScenario ORDER_BY_ADDITIONAL_FIELD = {
   .name = "ORDER_BY_ADDITIONAL_FIELD",
   .query = nlohmann::json::parse(R"(
{
  "action": {
    "sequenceNames": [
      "unaligned_segment1",
      "unaligned_segment2"
    ],
    "additionalFields": [
      "date"
    ],
    "orderByFields": [
      {
        "field": "date",
        "order": "ascending"
      }
    ],
    "type": "Fasta"
  },
  "filterExpression": {
    "type": "True"
  }
}
)"),
   .expected_query_result = nlohmann::json::parse(R"(
[{"date":"2024-08-01","primaryKey":"bothSegments","unaligned_segment1":"A","unaligned_segment2":"G"},
{"date":"2024-08-02","primaryKey":"onlySegment2","unaligned_segment1":null,"unaligned_segment2":"T"},
{"date":"2024-08-02","primaryKey":"3","unaligned_segment1":null,"unaligned_segment2":"AA"},
{"date":"2024-08-03","primaryKey":"onlySegment1","unaligned_segment1":"T","unaligned_segment2":null},
{"date":"2024-08-03","primaryKey":"2","unaligned_segment1":null,"unaligned_segment2":null},
{"date":"2024-08-05","primaryKey":"1","unaligned_segment1":null,"unaligned_segment2":"A"},
{"date":"2024-08-08","primaryKey":"noSegment","unaligned_segment1":null,"unaligned_segment2":null}])"
   )
};

}  // namespace

QUERY_TEST(
   FastaTest,
   TEST_DATA,
   ::testing::Values(
      SEQUENCE_WITH_BOTH_SEGMENTS_SCENARIO,
      SEQUENCE_WITH_ONLY_FIRST_SEGMENT_SCENARIO,
      SEQUENCE_WITH_ONLY_SECOND_SEGMENT_SCENARIO,
      SEQUENCE_WITH_NO_SEGMENT_SCENARIO,
      DOWNLOAD_ALL_SEQUENCES_SCENARIO,
      DOWNLOAD_ALL_DATA,
      ORDER_BY_NOT_IN_OUTPUT,
      ORDER_BY_ADDITIONAL_FIELD,
      DUPLICATE_FIELDS
   )
);
