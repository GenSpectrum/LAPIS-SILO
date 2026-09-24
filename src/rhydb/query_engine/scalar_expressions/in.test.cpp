#include <string>

#include <nlohmann/json.hpp>

#include "rhydb/test/query_fixture.test.h"

namespace {
using rhydb::ReferenceGenomes;
using rhydb::test::QueryTestData;
using rhydb::test::QueryTestScenario;

nlohmann::json createData(
   const std::string& primary_key,
   const std::string& country,
   const std::string& region,
   int32_t year,
   double score
) {
   return {
      {"primaryKey", primary_key},
      {"country", country},
      {"region", region},
      {"year", year},
      {"score", score}
   };
}

// `country` is dictionary-encoded, `region` a plain string, `year` an int, `score` a float.
const auto DATABASE_CONFIG =
   R"(
schema:
  instanceName: "test"
  metadata:
   - name: "primaryKey"
     type: "string"
   - name: "country"
     type: "string"
     generateIndex: true
   - name: "region"
     type: "string"
   - name: "year"
     type: "int"
   - name: "score"
     type: "float"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{{}, {}};

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {
         createData("id_0", "Germany", "Europe", 2020, 1e-7),
         createData("id_1", "France", "Europe", 2021, 2e-7),
         createData("id_2", "Japan", "Asia", 2022, 3e-7),
         createData("id_3", "Germany", "Europe", 2020, 0.5),
         createData("id_4", "Brazil", "SouthAmerica", 2023, 1.0),
      },
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

const QueryTestScenario IN_SET_LITERAL_STRING = {
   .name = "IN_SET_LITERAL_STRING",
   .query = "default.filter(country.in({'Japan', 'Brazil'})).project({primaryKey})",
   .expected_query_result =
      nlohmann::json::parse(R"([{"primaryKey":"id_2"},{"primaryKey":"id_4"}])")
};

// A set literal of integers on an int column.
const QueryTestScenario IN_SET_LITERAL_INT_COLUMN = {
   .name = "IN_SET_LITERAL_INT_COLUMN",
   .query = "default.filter(year.in({2020, 2023})).project({primaryKey})",
   .expected_query_result =
      nlohmann::json::parse(R"([{"primaryKey":"id_0"},{"primaryKey":"id_3"},{"primaryKey":"id_4"}])"
      )
};

// A set literal of floats on a float column.
const QueryTestScenario IN_SET_LITERAL_FLOAT_COLUMN = {
   .name = "IN_SET_LITERAL_FLOAT_COLUMN",
   .query = "default.filter(score.in({0.0000002, 0.5})).project({primaryKey})",
   .expected_query_result =
      nlohmann::json::parse(R"([{"primaryKey":"id_1"},{"primaryKey":"id_3"}])")
};

}  // namespace

QUERY_TEST(
   InTest,
   TEST_DATA,
   ::testing::Values(IN_SET_LITERAL_STRING, IN_SET_LITERAL_INT_COLUMN, IN_SET_LITERAL_FLOAT_COLUMN)
)
