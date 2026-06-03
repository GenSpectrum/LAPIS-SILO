#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

nlohmann::json createData(const std::string& primaryKey, int value) {
   return {
      {"primaryKey", primaryKey},
      {"int_value", value},
      {"segment1", nullptr},
      {"gene1", nullptr},
      {"unaligned_segment1", nullptr}
   };
}

const std::vector<nlohmann::json> DATA = {createData("id_0", 1), createData("id_1", 2)};

const auto DATABASE_CONFIG =
   R"(
defaultNucleotideSequence: "segment1"
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
    - name: "int_value"
      type: "int"
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

// Assigns a literal of each supported type to a new column.
const QueryTestScenario MAP_LITERALS_SCENARIO = {
   .name = "MAP_LITERALS",
   .query =
      R"(default.map({a := 3, b := 1.5, c := 'hello', d := true}).project({primaryKey, a, b, c, d}))",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"a", 3}, {"b", 1.5}, {"c", "hello"}, {"d", true}},
       {{"primaryKey", "id_1"}, {"a", 3}, {"b", 1.5}, {"c", "hello"}, {"d", true}}}
   )
};

// An assignment whose name matches an existing column replaces it in place.
const QueryTestScenario MAP_OVERRIDE_SCENARIO = {
   .name = "MAP_OVERRIDE",
   .query = "default.map({int_value := 42}).project({primaryKey, int_value})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"int_value", 42}}, {{"primaryKey", "id_1"}, {"int_value", 42}}}
   )
};

const QueryTestScenario MAP_INT64_SCENARIO = {
   .name = "MAP_INT64",
   .query = "default.map({x := 3000000000}).project({primaryKey, x})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"x", 3000000000}}, {{"primaryKey", "id_1"}, {"x", 3000000000}}}
   )
};

// All projected columns are produced by the map; none are passed through from
// the child. The table scan must still emit one row per input record (a prior
// bug narrowed the scan to zero fields and returned no rows).
const QueryTestScenario MAP_ONLY_MAPPED_COLUMN_SCENARIO = {
   .name = "MAP_ONLY_MAPPED_COLUMN",
   .query = "default.map({a := 1}).project({a})",
   .expected_query_result = nlohmann::json({{{"a", 1}}, {{"a", 1}}})
};

}  // namespace

QUERY_TEST(
   MapTest,
   TEST_DATA,
   ::testing::Values(
      MAP_LITERALS_SCENARIO,
      MAP_OVERRIDE_SCENARIO,
      MAP_INT64_SCENARIO,
      MAP_ONLY_MAPPED_COLUMN_SCENARIO
   )
);
