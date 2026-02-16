#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

namespace {
const std::string DATE_2020 = "2020-12-24";
const std::string DATE_2021 = "2021-06-15";
const std::string DATE_2023 = "2023-01-20";

const nlohmann::json DATA_ROW1 = {
   {"primaryKey", "row1"},
   {"sorted_date", DATE_2020},
   {"unsorted_date", DATE_2023},
   {"segment1", nullptr},
   {"unaligned_segment1", nullptr},
   {"gene1", nullptr}
};

const nlohmann::json DATA_ROW2 = {
   {"primaryKey", "row2"},
   {"sorted_date", DATE_2021},
   {"unsorted_date", DATE_2020},
   {"segment1", nullptr},
   {"unaligned_segment1", nullptr},
   {"gene1", nullptr}
};

const nlohmann::json DATA_ROW3 = {
   {"primaryKey", "row3"},
   {"sorted_date", DATE_2020},
   {"unsorted_date", DATE_2021},
   {"segment1", nullptr},
   {"unaligned_segment1", nullptr},
   {"gene1", nullptr}
};

const nlohmann::json DATA_NULL1 = {
   {"primaryKey", "null1"},
   {"sorted_date", nullptr},
   {"unsorted_date", nullptr},
   {"segment1", nullptr},
   {"unaligned_segment1", nullptr},
   {"gene1", nullptr}
};

const nlohmann::json DATA_NULL2 = {
   {"primaryKey", "null2"},
   {"sorted_date", nullptr},
   {"unsorted_date", DATE_2023},
   {"segment1", nullptr},
   {"unaligned_segment1", nullptr},
   {"gene1", nullptr}
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
   .ndjson_input_data = {DATA_ROW1, DATA_ROW2, DATA_ROW3, DATA_NULL1, DATA_NULL2},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

nlohmann::json createDateEqualsQuery(const std::string& column, const nlohmann::json& value) {
   return {
      {"action", {{"type", "Details"}}},
      {"filterExpression", {{"type", "DateEquals"}, {"column", column}, {"value", value}}}
   };
}

// Matches row1 and row3 (both have sorted_date = 2020-12-24)
const QueryTestScenario SORTED_DATE_MULTIPLE_MATCHES =
   {.name = "SORTED_DATE_MULTIPLE_MATCHES",
    .query = createDateEqualsQuery("sorted_date", DATE_2020),
    .expected_query_result = {
       {{"primaryKey", "row1"}, {"sorted_date", DATE_2020}, {"unsorted_date", DATE_2023}},
       {{"primaryKey", "row3"}, {"sorted_date", DATE_2020}, {"unsorted_date", DATE_2021}},
    }};

// Matches only row2 (sorted_date = 2021-06-15)
const QueryTestScenario SORTED_DATE_SINGLE_MATCH =
   {.name = "SORTED_DATE_SINGLE_MATCH",
    .query = createDateEqualsQuery("sorted_date", DATE_2021),
    .expected_query_result = {
       {{"primaryKey", "row2"}, {"sorted_date", DATE_2021}, {"unsorted_date", DATE_2020}},
    }};

// Matches row1 and null2 (both have unsorted_date = 2023-01-20)
const QueryTestScenario UNSORTED_DATE_MULTIPLE_MATCHES =
   {.name = "UNSORTED_DATE_MULTIPLE_MATCHES",
    .query = createDateEqualsQuery("unsorted_date", DATE_2023),
    .expected_query_result = {
       {{"primaryKey", "row1"}, {"sorted_date", DATE_2020}, {"unsorted_date", DATE_2023}},
       {{"primaryKey", "null2"}, {"sorted_date", nullptr}, {"unsorted_date", DATE_2023}},
    }};

// Matches only row2 (unsorted_date = 2020-12-24)
const QueryTestScenario UNSORTED_DATE_SINGLE_MATCH =
   {.name = "UNSORTED_DATE_SINGLE_MATCH",
    .query = createDateEqualsQuery("unsorted_date", DATE_2020),
    .expected_query_result = {
       {{"primaryKey", "row2"}, {"sorted_date", DATE_2021}, {"unsorted_date", DATE_2020}},
    }};

// Matches null1 and null2 (both have sorted_date = null)
const QueryTestScenario SORTED_DATE_NULL =
   {.name = "SORTED_DATE_NULL",
    .query = createDateEqualsQuery("sorted_date", nullptr),
    .expected_query_result = {
       {{"primaryKey", "null1"}, {"sorted_date", nullptr}, {"unsorted_date", nullptr}},
       {{"primaryKey", "null2"}, {"sorted_date", nullptr}, {"unsorted_date", DATE_2023}},
    }};

// Matches only null1 (unsorted_date = null)
const QueryTestScenario UNSORTED_DATE_NULL =
   {.name = "UNSORTED_DATE_NULL",
    .query = createDateEqualsQuery("unsorted_date", nullptr),
    .expected_query_result = {
       {{"primaryKey", "null1"}, {"sorted_date", nullptr}, {"unsorted_date", nullptr}},
    }};

const QueryTestScenario DATE_EQUALS_NO_MATCH = {
   .name = "DATE_EQUALS_NO_MATCH",
   .query = createDateEqualsQuery("sorted_date", "1999-01-01"),
   .expected_query_result = nlohmann::json::array()
};

const QueryTestScenario DATE_EQUALS_COLUMN_NOT_IN_DB = {
   .name = "DATE_EQUALS_COLUMN_NOT_IN_DB",
   .query = createDateEqualsQuery("something_not_in_database", "2020-01-01"),
   .expected_error_message = "The database does not contain the column 'something_not_in_database'"
};

const QueryTestScenario DATE_EQUALS_WRONG_COLUMN_TYPE = {
   .name = "DATE_EQUALS_WRONG_COLUMN_TYPE",
   .query = createDateEqualsQuery("primaryKey", "2020-01-01"),
   .expected_error_message = "The column 'primaryKey' is not of type date"
};

const QueryTestScenario DATE_EQUALS_WRONG_FORMAT = {
   .name = "DATE_EQUALS_WRONG_FORMAT",
   .query = createDateEqualsQuery("primaryKey", "2021-03-00018"),
   .expected_error_message =
      "The value for the DateEquals expression is not a valid date: Invalid date format "
      "'2021-03-00018': expected exactly YYYY-MM-DD"
};

const QueryTestScenario DATE_EQUALS_WRONG_VALUE_TYPE = {
   .name = "DATE_EQUALS_WRONG_VALUE_TYPE",
   .query = createDateEqualsQuery("primaryKey", "asdf"),
   .expected_error_message =
      "The value for the DateEquals expression is not a valid date: Invalid date format 'asdf': "
      "expected exactly YYYY-MM-DD"
};

}  // namespace

QUERY_TEST(
   DateEqualsTest,
   TEST_DATA,
   ::testing::Values(
      SORTED_DATE_MULTIPLE_MATCHES,
      SORTED_DATE_SINGLE_MATCH,
      UNSORTED_DATE_MULTIPLE_MATCHES,
      UNSORTED_DATE_SINGLE_MATCH,
      SORTED_DATE_NULL,
      UNSORTED_DATE_NULL,
      DATE_EQUALS_NO_MATCH,
      DATE_EQUALS_COLUMN_NOT_IN_DB,
      DATE_EQUALS_WRONG_COLUMN_TYPE,
      DATE_EQUALS_WRONG_FORMAT,
      DATE_EQUALS_WRONG_VALUE_TYPE
   )
);
