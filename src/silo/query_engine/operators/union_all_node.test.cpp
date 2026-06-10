#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

nlohmann::json createData(const std::string& primaryKey, const std::string& country) {
   return {
      {"primaryKey", primaryKey},
      {"country", country},
      {"segment1", nullptr},
      {"gene1", nullptr},
      {"unaligned_segment1", nullptr}
   };
}

const std::vector<nlohmann::json> DATA = {
   createData("id_0", "CH"),
   createData("id_1", "DE"),
   createData("id_2", "CH"),
   createData("id_3", "DE"),
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

// Basic unionAll of two filtered pipelines from the same table
const QueryTestScenario UNION_ALL_BASIC_SCENARIO = {
   .name = "UNION_ALL_BASIC",
   .query = R"(unionAll(
      default.filter(country='CH').project({primaryKey, country}),
      default.filter(country='DE').project({primaryKey, country})
   ))",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"country", "CH"}},
       {{"primaryKey", "id_2"}, {"country", "CH"}},
       {{"primaryKey", "id_1"}, {"country", "DE"}},
       {{"primaryKey", "id_3"}, {"country", "DE"}}}
   )
};

// UnionAll produces duplicates (all rows from both sides)
const QueryTestScenario UNION_ALL_DUPLICATES_SCENARIO = {
   .name = "UNION_ALL_DUPLICATES",
   .query = R"(unionAll(
      default.project({primaryKey}),
      default.project({primaryKey})
   ))",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}},
       {{"primaryKey", "id_1"}},
       {{"primaryKey", "id_2"}},
       {{"primaryKey", "id_3"}},
       {{"primaryKey", "id_0"}},
       {{"primaryKey", "id_1"}},
       {{"primaryKey", "id_2"}},
       {{"primaryKey", "id_3"}}}
   )
};

// UnionAll with downstream operations (groupBy on the result)
const QueryTestScenario UNION_ALL_WITH_GROUPBY_SCENARIO = {
   .name = "UNION_ALL_WITH_GROUPBY",
   .query = R"(unionAll(
      default.filter(country='CH').project({country}),
      default.filter(country='DE').project({country})
   ).groupBy({count := count()}, {country}).orderBy({asc(country)}))",
   .expected_query_result = nlohmann::json(
      {{{"country", "CH"}, {"count", 2}},
       {{"country", "DE"}, {"count", 2}}}
   )
};

// UnionAll where one child produces empty results
const QueryTestScenario UNION_ALL_EMPTY_CHILD_SCENARIO = {
   .name = "UNION_ALL_EMPTY_CHILD",
   .query = R"(unionAll(
      default.filter(country='CH').project({primaryKey, country}),
      default.filter(country='XX').project({primaryKey, country})
   ))",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"country", "CH"}},
       {{"primaryKey", "id_2"}, {"country", "CH"}}}
   )
};

// UnionAll with schema mismatch should error
const QueryTestScenario UNION_ALL_SCHEMA_MISMATCH_SCENARIO = {
   .name = "UNION_ALL_SCHEMA_MISMATCH",
   .query = R"(unionAll(
      default.project({primaryKey}),
      default.project({country})
   ))",
   .expected_query_result = {},
   .expected_error_message =
      "unionAll requires both inputs to have the same schema "
      "(same column names and types). "
      "Left schema: [primaryKey], right schema: [country]."
};
}  // namespace

QUERY_TEST(
   UnionAllTest,
   TEST_DATA,
   ::testing::Values(
      UNION_ALL_BASIC_SCENARIO,
      UNION_ALL_DUPLICATES_SCENARIO,
      UNION_ALL_WITH_GROUPBY_SCENARIO,
      UNION_ALL_EMPTY_CHILD_SCENARIO,
      UNION_ALL_SCHEMA_MISMATCH_SCENARIO
   )
);
