#include <nlohmann/json.hpp>

#include <optional>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

static const std::string SORTED_DATE_VALUE = "2020-12-24";
static const std::string UNSORTED_DATE_VALUE = "2023-01-20";

const nlohmann::json DATA = {
   {"metadata",
    {{"primaryKey", "id"},
     {"sorted_date", SORTED_DATE_VALUE},
     {"unsorted_date", UNSORTED_DATE_VALUE}}},
   {"alignedNucleotideSequences", {{"segment1", nullptr}}},
   {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
   {"alignedAminoAcidSequences", {{"gene1", nullptr}}}
};

const auto DATABASE_CONFIG = DatabaseConfig{
   "segment1",
   {"dummy name",
    {{"primaryKey", ValueType::STRING},
     {"sorted_date", ValueType::DATE},
     {"unsorted_date", ValueType::DATE}},
    "primaryKey",
    "sorted_date"}
};

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{"gene1", "*"}},
};

const QueryTestData TEST_DATA{{DATA}, DATABASE_CONFIG, REFERENCE_GENOMES};

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
   "sortedDateWithToEqualsFrom",
   createDateBetweenQuery("sorted_date", SORTED_DATE_VALUE, SORTED_DATE_VALUE),
   EXPECTED_RESULT
};

const QueryTestScenario SORTED_DATE_WITH_TO_ONLY_SCENARIO = {
   "sortedDateWithToOnly",
   createDateBetweenQuery("sorted_date", nullptr, SORTED_DATE_VALUE),
   EXPECTED_RESULT
};

const QueryTestScenario SORTED_DATE_WITH_FROM_ONLY_SCENARIO = {
   "sortedDateWithFromOnly",
   createDateBetweenQuery("sorted_date", SORTED_DATE_VALUE, nullptr),
   EXPECTED_RESULT
};

const QueryTestScenario UNSORTED_DATE_WITH_TO_AND_FROM_SCENARIO = {
   "unsortedDateWithToEqualsFrom",
   createDateBetweenQuery("unsorted_date", UNSORTED_DATE_VALUE, UNSORTED_DATE_VALUE),
   EXPECTED_RESULT
};

const QueryTestScenario UNSORTED_DATE_WITH_TO_ONLY_SCENARIO = {
   "unsortedDateWithToOnly",
   createDateBetweenQuery("unsorted_date", nullptr, UNSORTED_DATE_VALUE),
   EXPECTED_RESULT
};

const QueryTestScenario UNSORTED_DATE_WITH_FROM_ONLY_SCENARIO = {
   "unsortedDateWithFromOnly",
   createDateBetweenQuery("unsorted_date", UNSORTED_DATE_VALUE, nullptr),
   EXPECTED_RESULT
};

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
