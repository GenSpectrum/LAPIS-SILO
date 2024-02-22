#include <nlohmann/json.hpp>

#include <optional>

#include "silo/test/query_fixture.test.h"

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
      "alignedAminoAcidSequences": {"gene1": null}
   },
   {
      "metadata": {"key": "id2", "col": "B"},
      "alignedNucleotideSequences": {"segment1": null},
      "unalignedNucleotideSequences": {"segment1": null},
      "alignedAminoAcidSequences": {"gene1": null}
   },
   {
      "metadata": {"key": "id3", "col": "A"},
      "alignedNucleotideSequences": {"segment1": null},
      "unalignedNucleotideSequences": {"segment1": null},
      "alignedAminoAcidSequences": {"gene1": null}
   },
   {
      "metadata": {"key": "id4", "col": "B"},
      "alignedNucleotideSequences": {"segment1": null},
      "unalignedNucleotideSequences": {"segment1": null},
      "alignedAminoAcidSequences": {"gene1": null}
   },
   {
      "metadata": {"key": "id5", "col": "A"},
      "alignedNucleotideSequences": {"segment1": null},
      "unalignedNucleotideSequences": {"segment1": null},
      "alignedAminoAcidSequences": {"gene1": null}
   }
])";

// Parsing the JSON string to a json object
const std::vector<json> DATA = json::parse(DATA_JSON);

const auto DATABASE_CONFIG = DatabaseConfig{
   "segment1",
   {"dummy name", {{"key", ValueType::STRING}, {"col", ValueType::STRING}}, "key"}
};

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{"gene1", "*"}},
};

const QueryTestData TEST_DATA{DATA, DATABASE_CONFIG, REFERENCE_GENOMES};

const QueryTestScenario RANDOMIZE_SEED = {
   "seed1231ProvidedShouldShuffleResults",
   json::parse(
      R"({"action": {"type": "Details", "fields": ["key"], "randomize": {"seed": 1231}},
         "filterExpression": {"type": "True"}})"
   ),
   json::parse(
      R"([{"key": "id4"},
          {"key": "id1"},
          {"key": "id5"},
          {"key": "id2"},
          {"key": "id3"}])"
   )
};

const QueryTestScenario RANDOMIZE_SEED_DIFFERENT = {
   "seed12312ProvidedShouldShuffleResultsDifferently",
   json::parse(
      R"({"action": {"type": "Details", "fields": ["key"], "randomize": {"seed": 12312}},
         "filterExpression": {"type": "True"}})"
   ),
   json::parse(
      R"([{"key": "id1"},
          {"key": "id4"},
          {"key": "id3"},
          {"key": "id2"},
          {"key": "id5"}])"
   )
};

const QueryTestScenario EXPLICIT_DO_NOT_RANDOMIZE = {
   "explicitlyDoNotRandomize",
   json::parse(
      R"({"action": {"type": "Details", "fields": ["key"], "randomize": false},
         "filterExpression": {"type": "True"}})"
   ),
   json::parse(
      R"([{"key": "id1"},
          {"key": "id2"},
          {"key": "id3"},
          {"key": "id4"},
          {"key": "id5"}])"
   )
};

const QueryTestScenario AGGREGATE = {
   "aggregateRandomize",
   json::parse(
      R"({"action": {"type": "Aggregated", "groupByFields": ["key"], "randomize": {"seed": 12321}},
         "filterExpression": {"type": "True"}})"
   ),
   json::parse(
      R"([{"count": 1, "key": "id3"},
          {"count": 1, "key": "id1"},
          {"count": 1, "key": "id4"},
          {"count": 1, "key": "id5"},
          {"count": 1, "key": "id2"}])"
   )
};

const QueryTestScenario ORDER_BY_PRECEDENCE = {
   "orderByTakePrecedenceOverRandomize",
   json::parse(
      R"({"action": {"type": "Details", "fields": ["key", "col"], "randomize": {"seed": 123212}, "orderByFields": ["col"]},
         "filterExpression": {"type": "True"}})"
   ),
   json::parse(
      R"([{"key": "id5", "col": "A"},
          {"key": "id1", "col": "A"},
          {"key": "id3", "col": "A"},
          {"key": "id2", "col": "B"},
          {"key": "id4", "col": "B"}])"
   )
};

const QueryTestScenario ORDER_BY_AGGREGATE_RANDOMIZE = {
   "orderingByAggregatedCount",
   json::parse(
      R"({"action": {"type": "Aggregated", "groupByFields": ["col"], "randomize": true, "orderByFields": ["count"]},
         "filterExpression": {"type": "True"}})"
   ),
   json::parse(
      R"([{"count": 2, "col": "B"},
          {"count": 3, "col": "A"}])"
   )
};

const QueryTestScenario LIMIT_2_RANDOMIZE = {
   "detailsWithLimit2AndOffsetRandomized",
   json::parse(
      R"({"action": {"type": "Details", "fields": ["key", "col"],
                     "orderByFields": ["col", "key"], "limit": 2, "offset": 2},
         "filterExpression": {"type": "True"}})"
   ),
   json::parse(
      R"([{"key": "id5", "col": "A"},
          {"key": "id2", "col": "B"}])"
   )
};

const QueryTestScenario LIMIT_3_RANDOMIZE = {
   "detailsWithLimit3AndOffsetRandomized",
   json::parse(
      R"({"action": {"type": "Details", "fields": ["key", "col"],
                     "orderByFields": ["col", "key"], "limit": 3, "offset": 2},
         "filterExpression": {"type": "True"}})"
   ),
   json::parse(
      R"([{"key": "id5", "col": "A"},
          {"key": "id2", "col": "B"},
          {"key": "id4", "col": "B"}])"
   )
};

const QueryTestScenario AGGREGATE_LIMIT_RANDOMIZE = {
   "aggregateWithLimitAndOffsetRandomized",
   json::parse(
      R"({"action": {"type": "Aggregated", "groupByFields": ["key", "col"], "randomize": {"seed": 123},
                     "orderByFields": ["col"], "limit": 2, "offset": 1},
         "filterExpression": {"type": "True"}})"
   ),
   json::parse(
      R"([{"count": 1, "key": "id1", "col": "A"},
          {"count": 1, "key": "id3", "col": "A"}])"
   )
};

QUERY_TEST(
   RandomizeTest,
   TEST_DATA,
   ::testing::Values(
      RANDOMIZE_SEED,
      RANDOMIZE_SEED_DIFFERENT,
      EXPLICIT_DO_NOT_RANDOMIZE,
      AGGREGATE,
      ORDER_BY_PRECEDENCE,
      ORDER_BY_AGGREGATE_RANDOMIZE,
      LIMIT_2_RANDOMIZE,
      LIMIT_3_RANDOMIZE,
      AGGREGATE_LIMIT_RANDOMIZE
   )
);
