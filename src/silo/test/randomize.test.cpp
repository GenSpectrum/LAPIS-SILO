#include <nlohmann/json.hpp>

#include <optional>

#include "silo/test/query_fixture.test.h"

namespace {
using nlohmann::json;

using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

const auto DATA_JSON = R"([
   {
      "metadata": {"key": "id1", "col": "A"},
      "alignedNucleotideSequences": {"segment1": null},
      "unalignedNucleotideSequences": {"segment1": null},
      "alignedAminoAcidSequences": {"gene1": null},
      "nucleotideInsertions": {"segment1": []},
      "aminoAcidInsertions": {"gene1": []}
   },
   {
      "metadata": {"key": "id2", "col": "B"},
      "alignedNucleotideSequences": {"segment1": null},
      "unalignedNucleotideSequences": {"segment1": null},
      "alignedAminoAcidSequences": {"gene1": null},
      "nucleotideInsertions": {"segment1": []},
      "aminoAcidInsertions": {"gene1": []}
   },
   {
      "metadata": {"key": "id3", "col": "A"},
      "alignedNucleotideSequences": {"segment1": null},
      "unalignedNucleotideSequences": {"segment1": null},
      "alignedAminoAcidSequences": {"gene1": null},
      "nucleotideInsertions": {"segment1": []},
      "aminoAcidInsertions": {"gene1": []}
   },
   {
      "metadata": {"key": "id4", "col": "B"},
      "alignedNucleotideSequences": {"segment1": null},
      "unalignedNucleotideSequences": {"segment1": null},
      "alignedAminoAcidSequences": {"gene1": null},
      "nucleotideInsertions": {"segment1": []},
      "aminoAcidInsertions": {"gene1": []}
   },
   {
      "metadata": {"key": "id5", "col": "A"},
      "alignedNucleotideSequences": {"segment1": null},
      "unalignedNucleotideSequences": {"segment1": null},
      "alignedAminoAcidSequences": {"gene1": null},
      "nucleotideInsertions": {"segment1": []},
      "aminoAcidInsertions": {"gene1": []}
   }
])";

const std::vector<json> DATA = json::parse(DATA_JSON);

const auto DATABASE_CONFIG =
   R"(
defaultNucleotideSequence: "segment1"
schema:
  instanceName: "dummy name"
  metadata:
    - name: "key"
      type: "string"
    - name: "col"
      type: "string"
  primaryKey: "key"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{"gene1", "*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = DATA,
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

const QueryTestScenario RANDOMIZE_SEED = {
   .name = "RANDOMIZE_SEED",
   .query = json::parse(
      R"({"action": {"type": "Details", "fields": ["key"], "randomize": {"seed": 1231}},
         "filterExpression": {"type": "True"}})"
   ),
   .expected_query_result = json::parse(
      R"([{"key": "id5"},
          {"key": "id1"},
          {"key": "id4"},
          {"key": "id2"},
          {"key": "id3"}])"
   )
};

const QueryTestScenario RANDOMIZE_INDEPENDENT_ON_COL_NUMS = {
   .name = "RANDOMIZE_INDEPENDENT_ON_COL_NUMS",
   .query = json::parse(
      R"({"action": {"type": "Details", "fields": ["key", "col"], "randomize": {"seed": 1231}},
         "filterExpression": {"type": "True"}})"
   ),
   .expected_query_result = json::parse(
      R"(
[{"col":"A","key":"id5"},
{"col":"A","key":"id1"},
{"col":"B","key":"id4"},
{"col":"B","key":"id2"},
{"col":"A","key":"id3"}]
)"
   )
};

const QueryTestScenario RANDOMIZE_INDEPENDENT_ON_BATCH_SIZE = {
   .name = "RANDOMIZE_INDEPENDENT_ON_BATCH_SIZE",
   .query = json::parse(
      R"({"action": {"type": "Details", "fields": ["key"], "randomize": {"seed": 1231}},
         "filterExpression": {"type": "True"}})"
   ),
   .expected_query_result = json::parse(
      R"([{"key": "id5"},
          {"key": "id1"},
          {"key": "id4"},
          {"key": "id2"},
          {"key": "id3"}])"
   ),
   .query_options = silo::config::QueryOptions{.materialization_cutoff = 2}
};

const QueryTestScenario DIFFERENT_RANDOMIZE_SEED_DIFFERENT_RESULT = {
   .name = "DIFFERENT_RANDOMIZE_SEED_DIFFERENT_RESULT",
   .query = json::parse(
      R"({"action": {"type": "Details", "fields": ["key"], "randomize": {"seed": 12312}},
         "filterExpression": {"type": "True"}})"
   ),
   .expected_query_result = json::parse(
      R"([{"key": "id1"},
          {"key": "id3"},
          {"key": "id5"},
          {"key": "id2"},
          {"key": "id4"}])"
   )
};

const QueryTestScenario EXPLICIT_DO_NOT_RANDOMIZE = {
   .name = "EXPLICIT_DO_NOT_RANDOMIZE",
   .query = json::parse(
      R"({"action": {"type": "Details", "fields": ["key"], "randomize": false},
         "filterExpression": {"type": "True"}})"
   ),
   .expected_query_result = json::parse(
      R"([{"key": "id1"},
          {"key": "id2"},
          {"key": "id3"},
          {"key": "id4"},
          {"key": "id5"}])"
   )
};

const QueryTestScenario AGGREGATE_RANDOMIZE = {
   .name = "AGGREGATE_RANDOMIZE",
   .query = json::parse(
      R"({"action": {"type": "Aggregated", "groupByFields": ["key"], "randomize": {"seed": 12321}},
         "filterExpression": {"type": "True"}})"
   ),
   .expected_query_result = json::parse(
      R"([
{"count": 1, "key": "id4"},
{"count": 1, "key": "id5"},
{"count": 1, "key": "id1"},
{"count": 1, "key": "id3"},
{"count": 1, "key": "id2"}
])"
   )
};

const QueryTestScenario ORDER_BY_PRECEDENCE = {
   .name = "orderByTakePrecedenceOverRandomize",
   .query = json::parse(
      R"({"action": {"type": "Details", "fields": ["key", "col"], "randomize": {"seed": 12321}, "orderByFields": ["col"]},
         "filterExpression": {"type": "True"}})"
   ),
   .expected_query_result = json::parse(
      R"([
{"key": "id5", "col": "A"},
{"key": "id1", "col": "A"},
{"key": "id3", "col": "A"},
{"key": "id4", "col": "B"},
{"key": "id2", "col": "B"}
])"
   )
};

const QueryTestScenario ORDER_BY_AGGREGATE_RANDOMIZE = {
   .name = "orderingByAggregatedCount",
   .query = json::parse(
      R"({"action": {"type": "Aggregated", "groupByFields": ["col"], "randomize": true, "orderByFields": ["count"]},
         "filterExpression": {"type": "True"}})"
   ),
   .expected_query_result = json::parse(
      R"([{"count": 2, "col": "B"},
          {"count": 3, "col": "A"}])"
   )
};

const QueryTestScenario LIMIT_2_RANDOMIZE = {
   .name = "detailsWithLimit2AndOffsetRandomized",
   .query = json::parse(
      R"({"action": {"type": "Details", "fields": ["key", "col"], "randomize": true,
                     "orderByFields": ["col", "key"], "limit": 2, "offset": 2},
         "filterExpression": {"type": "True"}})"
   ),
   .expected_query_result = json::parse(
      R"([{"key": "id5", "col": "A"},
          {"key": "id2", "col": "B"}])"
   )
};

const QueryTestScenario LIMIT_3_RANDOMIZE = {
   .name = "detailsWithLimit3AndOffsetRandomized",
   .query = json::parse(
      R"({"action": {"type": "Details", "fields": ["key", "col"], "randomize": true,
                     "orderByFields": ["col", "key"], "limit": 3, "offset": 2},
         "filterExpression": {"type": "True"}})"
   ),
   .expected_query_result = json::parse(
      R"([{"key": "id5", "col": "A"},
          {"key": "id2", "col": "B"},
          {"key": "id4", "col": "B"}])"
   )
};

const QueryTestScenario AGGREGATE_LIMIT_RANDOMIZE = {
   .name = "aggregateWithLimitAndOffsetRandomized",
   .query = json::parse(
      R"({"action": {"type": "Aggregated", "groupByFields": ["key"], "randomize": {"seed": 12321},
"limit": 2, "offset": 1},
         "filterExpression": {"type": "True"}})"
   ),
   .expected_query_result = json::parse(
      R"([{"count": 1, "key": "id5"},
          {"count": 1, "key": "id1"}])"
   )
};

}  // namespace

QUERY_TEST(
   RandomizeTest,
   TEST_DATA,
   ::testing::Values(
      RANDOMIZE_SEED,
      RANDOMIZE_INDEPENDENT_ON_COL_NUMS,
      RANDOMIZE_INDEPENDENT_ON_BATCH_SIZE,
      DIFFERENT_RANDOMIZE_SEED_DIFFERENT_RESULT,
      EXPLICIT_DO_NOT_RANDOMIZE,
      AGGREGATE_RANDOMIZE,
      ORDER_BY_PRECEDENCE,
      ORDER_BY_AGGREGATE_RANDOMIZE,
      LIMIT_2_RANDOMIZE,
      LIMIT_3_RANDOMIZE,
      AGGREGATE_LIMIT_RANDOMIZE
   )
);
