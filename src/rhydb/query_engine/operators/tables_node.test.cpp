#include <nlohmann/json.hpp>

#include "rhydb/test/query_fixture.test.h"

namespace {
using rhydb::ReferenceGenomes;
using rhydb::test::QueryTestData;
using rhydb::test::QueryTestScenario;

const std::vector<nlohmann::json> DATA = {
   {{"primaryKey", "id_0"},
    {"country", "CH"},
    {"segment1", nullptr},
    {"gene1", nullptr},
    {"unaligned_segment1", nullptr}},
   {{"primaryKey", "id_1"},
    {"country", "DE"},
    {"segment1", nullptr},
    {"gene1", nullptr},
    {"unaligned_segment1", nullptr}},
};

const auto DATABASE_CONFIG =
   R"(
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
    - name: "country"
      type: "string"
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

const QueryTestScenario TABLES_SCENARIO = {
   .name = "TABLES",
   .query = "tables()",
   .expected_query_result = nlohmann::json({{{"tableName", "default"}}}),
};

const QueryTestScenario TABLES_SCHEMA_SCENARIO = {
   .name = "TABLES_SCHEMA",
   .query = "tables().schema()",
   .expected_query_result = nlohmann::json({{{"fieldName", "tableName"}, {"type", "STRING"}}}),
};

const QueryTestScenario TABLES_EXTRA_ARG_ERROR_SCENARIO = {
   .name = "TABLES_EXTRA_ARG_ERROR",
   .query = "tables(default)",
   .expected_error_message = "tables() received too many positional arguments",
};

}  // namespace

QUERY_TEST(
   TablesTest,
   TEST_DATA,
   ::testing::Values(TABLES_SCENARIO, TABLES_SCHEMA_SCENARIO, TABLES_EXTRA_ARG_ERROR_SCENARIO)
);
