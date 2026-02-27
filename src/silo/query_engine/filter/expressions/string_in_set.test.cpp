#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

nlohmann::json createData(const std::string& primary_key, const std::string& country) {
   return nlohmann::json::parse(fmt::format(
      R"(
{{
   "primaryKey": "{}",
   "country": "{}",
   "region": "{}",
   "segment1": {{
      "sequence": "ACGT",
      "insertions": []
   }}
}}
)",
      primary_key,
      country,
      country == "Switzerland" ? "Europe" : (country == "USA" ? "Americas" : "Europe")
   ));
}

const auto DATABASE_CONFIG =
   R"(
defaultNucleotideSequence: "segment1"
schema:
  instanceName: "dummy name"
  metadata:
   - name: "primaryKey"
     type: "string"
   - name: "country"
     type: "string"
   - name: "region"
     type: "string"
     generateIndex: true
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "ATGCN"}},
   {},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {createData("id_0", "Switzerland"),
       createData("id_1", "Germany"),
       createData("id_2", "USA"),
       createData("id_3", "Switzerland"),
       createData("id_4", "France"),
       createData("id_5", "Germany")},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .without_unaligned_sequences = true
};

const QueryTestScenario STRING_IN_SET_SINGLE_VALUE = {
   .name = "STRING_IN_SET_SINGLE_VALUE",
   .query = "metadata.filter(country.in({'Switzerland'})).details(primaryKey, country)",
   .expected_query_result = nlohmann::json::parse(
      R"([{"country":"Switzerland","primaryKey":"id_0"},{"country":"Switzerland","primaryKey":"id_3"}])"
   )
};

const QueryTestScenario STRING_IN_SET_MULTIPLE_VALUES = {
   .name = "STRING_IN_SET_MULTIPLE_VALUES",
   .query = "metadata.filter(country.in({'Switzerland', 'Germany'})).details(primaryKey, country)",
   .expected_query_result = nlohmann::json::parse(
      R"([{"country":"Switzerland","primaryKey":"id_0"},{"country":"Germany","primaryKey":"id_1"},{"country":"Switzerland","primaryKey":"id_3"},{"country":"Germany","primaryKey":"id_5"}])"
   )
};

const QueryTestScenario STRING_IN_SET_NO_MATCH = {
   .name = "STRING_IN_SET_NO_MATCH",
   .query = "metadata.filter(country.in({'Japan', 'China'})).details(primaryKey, country)",
   .expected_query_result = nlohmann::json::parse(R"([])")
};

const QueryTestScenario STRING_IN_SET_EMPTY_VALUES = {
   .name = "STRING_IN_SET_EMPTY_VALUES",
   .query = "metadata.filter(country.in({})).details(primaryKey, country)",
   .expected_query_result = nlohmann::json::parse(R"([])")
};

const QueryTestScenario STRING_IN_SET_INDEXED_COLUMN = {
   .name = "STRING_IN_SET_INDEXED_COLUMN",
   .query = "metadata.filter(region.in({'Europe'})).details(primaryKey, region)",
   .expected_query_result = nlohmann::json::parse(
      R"([{"primaryKey":"id_0","region":"Europe"},{"primaryKey":"id_1","region":"Europe"},{"primaryKey":"id_3","region":"Europe"},{"primaryKey":"id_4","region":"Europe"},{"primaryKey":"id_5","region":"Europe"}])"
   )
};

const QueryTestScenario STRING_IN_SET_WITH_AND = {
   .name = "STRING_IN_SET_WITH_AND",
   .query =
      "metadata.filter(country.in({'Switzerland', 'Germany', 'France'}) && region = "
      "'Europe').details(primaryKey, country, region)",
   .expected_query_result = nlohmann::json::parse(
      R"([{"country":"Switzerland","primaryKey":"id_0","region":"Europe"},{"country":"Germany","primaryKey":"id_1","region":"Europe"},{"country":"Switzerland","primaryKey":"id_3","region":"Europe"},{"country":"France","primaryKey":"id_4","region":"Europe"},{"country":"Germany","primaryKey":"id_5","region":"Europe"}])"
   )
};

const QueryTestScenario STRING_IN_SET_NEGATED = {
   .name = "STRING_IN_SET_NEGATED",
   .query =
      "metadata.filter(!(country.in({'Switzerland', 'Germany'}))).details(primaryKey, country)",
   .expected_query_result = nlohmann::json::parse(
      R"([{"country":"USA","primaryKey":"id_2"},{"country":"France","primaryKey":"id_4"}])"
   )
};

const QueryTestScenario STRING_IN_SET_NONEXISTENT_COLUMN = {
   .name = "STRING_IN_SET_NONEXISTENT_COLUMN",
   .query = "metadata.filter(nonexistent.in({'Switzerland'})).details()",
   .expected_query_result = {},
   .expected_error_message = "The database does not contain the string column 'nonexistent'"
};

}  // namespace

QUERY_TEST(
   StringInSet,
   TEST_DATA,
   ::testing::Values(
      STRING_IN_SET_SINGLE_VALUE,
      STRING_IN_SET_MULTIPLE_VALUES,
      STRING_IN_SET_NO_MATCH,
      STRING_IN_SET_EMPTY_VALUES,
      STRING_IN_SET_INDEXED_COLUMN,
      STRING_IN_SET_WITH_AND,
      STRING_IN_SET_NEGATED,
      STRING_IN_SET_NONEXISTENT_COLUMN
   )
);
