#include <nlohmann/json.hpp>

#include <optional>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

namespace {
const std::string SORTED_DATE_VALUE = "2020-12-24";
const std::string UNSORTED_DATE_VALUE = "2023-01-20";

const nlohmann::json DATA = {
   {"metadata",
    {{"primaryKey", "id"},
     {"sorted_date", SORTED_DATE_VALUE},
     {"unsorted_date", UNSORTED_DATE_VALUE}}},
   {"alignedNucleotideSequences", {{"segment1", nullptr}}},
   {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
   {"alignedAminoAcidSequences", {{"gene1", nullptr}}},
   {"nucleotideInsertions", {{"segment1", {}}}},
   {"aminoAcidInsertions", {{"gene1", {}}}}
};

const auto DATABASE_CONFIG =
   R"(
defaultNucleotideSequence: "segment1"
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
    - name: "sorted_date"
      type: "date"
    - name: "unsorted_date"
      type: "date"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{"gene1", "*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = {DATA},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

nlohmann::json createDateBetweenQuery(
   const std::string& column,
   const nlohmann::json from_date,
   const nlohmann::json to_date
) {
   return {
      {"action", {{"type", "Details"}}},
      {"filterExpression",
       {{"type", "DateBetween"}, {"column", column}, {"from", from_date}, {"to", to_date}}}
   };
}

const nlohmann::json EXPECTED_RESULT = {
   {{"primaryKey", "id"}, {"sorted_date", SORTED_DATE_VALUE}, {"unsorted_date", UNSORTED_DATE_VALUE}
   }
};

const QueryTestScenario SORTED_DATE_WITH_TO_AND_FROM_SCENARIO = {
   .name = "sortedDateWithToEqualsFrom",
   .query = createDateBetweenQuery("sorted_date", SORTED_DATE_VALUE, SORTED_DATE_VALUE),
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario SORTED_DATE_WITH_TO_ONLY_SCENARIO = {
   .name = "sortedDateWithToOnly",
   .query = createDateBetweenQuery("sorted_date", nullptr, SORTED_DATE_VALUE),
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario SORTED_DATE_WITH_FROM_ONLY_SCENARIO = {
   .name = "sortedDateWithFromOnly",
   .query = createDateBetweenQuery("sorted_date", SORTED_DATE_VALUE, nullptr),
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario UNSORTED_DATE_WITH_TO_AND_FROM_SCENARIO = {
   .name = "unsortedDateWithToEqualsFrom",
   .query = createDateBetweenQuery("unsorted_date", UNSORTED_DATE_VALUE, UNSORTED_DATE_VALUE),
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario UNSORTED_DATE_WITH_TO_ONLY_SCENARIO = {
   .name = "unsortedDateWithToOnly",
   .query = createDateBetweenQuery("unsorted_date", nullptr, UNSORTED_DATE_VALUE),
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario UNSORTED_DATE_WITH_FROM_ONLY_SCENARIO = {
   .name = "unsortedDateWithFromOnly",
   .query = createDateBetweenQuery("unsorted_date", UNSORTED_DATE_VALUE, nullptr),
   .expected_query_result = EXPECTED_RESULT
};

}  // namespace

QUERY_TEST(
   DateBetweenTest,
   TEST_DATA,
   ::testing::Values(
      SORTED_DATE_WITH_TO_AND_FROM_SCENARIO,
      SORTED_DATE_WITH_TO_ONLY_SCENARIO,
      SORTED_DATE_WITH_FROM_ONLY_SCENARIO,
      UNSORTED_DATE_WITH_TO_AND_FROM_SCENARIO,
      UNSORTED_DATE_WITH_TO_ONLY_SCENARIO,
      UNSORTED_DATE_WITH_FROM_ONLY_SCENARIO
   )
);
