#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

nlohmann::json createData(
   const std::string& primaryKey,
   const std::string& country,
   const std::string& date,
   bool is_covered,
   int age,
   double proportion
) {
   return {
      {"primaryKey", primaryKey},
      {"country", country},
      {"date", date},
      {"is_covered", is_covered},
      {"age", age},
      {"proportion", proportion},
      {"segment1", {{"sequence", "T"}, {"insertions", nlohmann::json::array()}}},
      {"gene1", nullptr},
      {"unaligned_segment1", nullptr},
   };
}

const std::vector<nlohmann::json> DATA = {
   createData("id_0", "CH", "2021-01-01", true, 30, 0.5),
   createData("id_1", "DE", "2021-02-01", false, 40, 0.75),
   createData("id_2", "CH", "2021-03-01", true, 30, 0.5),
   createData("id_3", "DE", "2021-04-01", false, 50, 0.25),
};

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
    - name: "date"
      type: "date"
    - name: "is_covered"
      type: "boolean"
    - name: "age"
      type: "int"
    - name: "proportion"
      type: "float"
  primaryKey: "primaryKey"
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

const QueryTestScenario SCHEMA_OF_TABLE_SCENARIO = {
   .name = "SCHEMA_OF_TABLE",
   .query = R"(default.schema())",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "age"}, {"type", "INT32"}},
       {{"fieldName", "country"}, {"type", "STRING"}},
       {{"fieldName", "date"}, {"type", "DATE32"}},
       {{"fieldName", "gene1"}, {"type", "STRING"}},
       {{"fieldName", "is_covered"}, {"type", "BOOL"}},
       {{"fieldName", "primaryKey"}, {"type", "STRING"}},
       {{"fieldName", "proportion"}, {"type", "FLOAT"}},
       {{"fieldName", "segment1"}, {"type", "STRING"}},
       {{"fieldName", "unaligned_segment1"}, {"type", "STRING"}}}
   )
};

const QueryTestScenario SCHEMA_AFTER_FILTER_SCENARIO = {
   .name = "SCHEMA_AFTER_FILTER",
   .query = R"(default.filter(country='CH').schema())",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "age"}, {"type", "INT32"}},
       {{"fieldName", "country"}, {"type", "STRING"}},
       {{"fieldName", "date"}, {"type", "DATE32"}},
       {{"fieldName", "gene1"}, {"type", "STRING"}},
       {{"fieldName", "is_covered"}, {"type", "BOOL"}},
       {{"fieldName", "primaryKey"}, {"type", "STRING"}},
       {{"fieldName", "proportion"}, {"type", "FLOAT"}},
       {{"fieldName", "segment1"}, {"type", "STRING"}},
       {{"fieldName", "unaligned_segment1"}, {"type", "STRING"}}}
   )
};

const QueryTestScenario SCHEMA_AFTER_PROJECT_SCENARIO = {
   .name = "SCHEMA_AFTER_PROJECT",
   .query = R"(default.project({primaryKey, country, age}).schema())",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "primaryKey"}, {"type", "STRING"}},
       {{"fieldName", "country"}, {"type", "STRING"}},
       {{"fieldName", "age"}, {"type", "INT32"}}}
   )
};

const QueryTestScenario SCHEMA_AFTER_GROUPBY_SCENARIO = {
   .name = "SCHEMA_AFTER_GROUPBY",
   .query = R"(default.groupBy({count:=count()}, {age}).schema())",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "age"}, {"type", "INT32"}},
       {{"fieldName", "count"}, {"type", "INT64"}}}
   )
};

const QueryTestScenario SCHEMA_AFTER_MAP_SCENARIO = {
   .name = "SCHEMA_AFTER_MAP",
   .query = R"(default.map({x := 42, label := 'hello'}).project({primaryKey, x, label}).schema())",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "primaryKey"}, {"type", "STRING"}},
       {{"fieldName", "x"}, {"type", "INT64"}},
       {{"fieldName", "label"}, {"type", "STRING"}}}
   )
};

const QueryTestScenario SCHEMA_AFTER_MUTATIONS_SCENARIO = {
   .name = "SCHEMA_AFTER_MUTATIONS",
   .query =
      R"(default.mutations(minProportion:=0.0, fields:={mutation, position, count}).schema())",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "mutation"}, {"type", "STRING"}},
       {{"fieldName", "position"}, {"type", "INT32"}},
       {{"fieldName", "count"}, {"type", "INT32"}}}
   )
};

}  // namespace

QUERY_TEST(
   SchemaNodeTest,
   TEST_DATA,
   ::testing::Values(
      SCHEMA_OF_TABLE_SCENARIO,
      SCHEMA_AFTER_FILTER_SCENARIO,
      SCHEMA_AFTER_PROJECT_SCENARIO,
      SCHEMA_AFTER_GROUPBY_SCENARIO,
      SCHEMA_AFTER_MAP_SCENARIO,
      SCHEMA_AFTER_MUTATIONS_SCENARIO
   )
);
