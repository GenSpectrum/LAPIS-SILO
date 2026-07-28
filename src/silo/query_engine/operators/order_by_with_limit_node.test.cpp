#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

nlohmann::json createData(
   const std::string& primaryKey,
   const nlohmann::json& int_value,
   const nlohmann::json& date_value
) {
   return {
      {"primaryKey", primaryKey},
      {"int_value", int_value},
      {"segment1", nullptr},
      {"gene1", nullptr},
      {"unaligned_segment1", nullptr},
      {"date", date_value}
   };
}

// Same data as the OrderByNode test: both sort columns contain nulls so that top-k selection can
// be checked to place null/missing as the smallest element, just like the full-sort path it
// replaces.
const std::vector<nlohmann::json> DATA = {
   createData("id_0", nullptr, nullptr),
   createData("id_1", nullptr, "2023-01-01"),
   createData("id_2", 1, nullptr),
   createData("id_3", 1, "2023-01-01"),
   createData("id_4", 2, nullptr),
   createData("id_5", 2, "2023-01-01")
};

const auto DATABASE_CONFIG =
   R"(
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
    - name: "int_value"
      type: "int"
    - name: "date"
      type: "date"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "ACGT"}},
   {{"gene1", "*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = DATA,
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

// orderBy ascending + limit: the three smallest rows, nulls (smallest) first.
const QueryTestScenario ASC_LIMIT_SCENARIO = {
   .name = "ORDER_BY_WITH_LIMIT_ASC",
   .query =
      "default.project({primaryKey, int_value, date}).orderBy({int_value.asc(), "
      "date.asc()}).limit(3)",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"int_value", nullptr}, {"date", nullptr}},
       {{"primaryKey", "id_1"}, {"int_value", nullptr}, {"date", "2023-01-01"}},
       {{"primaryKey", "id_2"}, {"int_value", 1}, {"date", nullptr}}}
   )
};

// orderBy descending + limit: the three largest rows, nulls (smallest) sort last so they do not
// appear in the top-k at all.
const QueryTestScenario DESC_LIMIT_SCENARIO = {
   .name = "ORDER_BY_WITH_LIMIT_DESC",
   .query =
      "default.project({primaryKey, int_value, date}).orderBy({int_value.desc(), "
      "date.desc()}).limit(3)",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_5"}, {"int_value", 2}, {"date", "2023-01-01"}},
       {{"primaryKey", "id_4"}, {"int_value", 2}, {"date", nullptr}},
       {{"primaryKey", "id_3"}, {"int_value", 1}, {"date", "2023-01-01"}}}
   )
};

// A limit larger than the input returns every row, still fully ordered.
const QueryTestScenario LIMIT_LARGER_THAN_INPUT_SCENARIO = {
   .name = "ORDER_BY_WITH_LIMIT_LARGER_THAN_INPUT",
   .query =
      "default.project({primaryKey, date}).orderBy({date.asc(), primaryKey.asc()}).limit(100)",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"date", nullptr}},
       {{"primaryKey", "id_2"}, {"date", nullptr}},
       {{"primaryKey", "id_4"}, {"date", nullptr}},
       {{"primaryKey", "id_1"}, {"date", "2023-01-01"}},
       {{"primaryKey", "id_3"}, {"date", "2023-01-01"}},
       {{"primaryKey", "id_5"}, {"date", "2023-01-01"}}}
   )
};

}  // namespace

QUERY_TEST(
   OrderByWithLimitNodeTest,
   TEST_DATA,
   ::testing::Values(ASC_LIMIT_SCENARIO, DESC_LIMIT_SCENARIO, LIMIT_LARGER_THAN_INPUT_SCENARIO)
);
