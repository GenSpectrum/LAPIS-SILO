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
   int age,
   const nlohmann::json& segment1 = nullptr
) {
   return {
      {"primaryKey", primaryKey},
      {"country", country},
      {"date", date},
      {"age", age},
      {"segment1", segment1},
      {"gene1", nullptr},
      {"unaligned_segment1", nullptr},
   };
}

nlohmann::json alignedSequence(const std::string& sequence) {
   return {{"sequence", sequence}, {"insertions", nlohmann::json::array()}};
}

const std::vector<nlohmann::json> DATA = {
   createData("id_0", "CH", "2024-01-01", 31, alignedSequence("ACGT")),
   createData("id_1", "DE", "2024-01-02", 42, alignedSequence("ACGA")),
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
    - name: "age"
      type: "int"
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

// schema() on a base table reports every output field of the scan. The queried
// sequence column `segment1` is decompressed to STRING, so it is reported as
// STRING rather than NUCLEOTIDE_SEQUENCE (documented limitation).
const QueryTestScenario TABLE_SCHEMA_SCENARIO = {
   .name = "TABLE_SCHEMA",
   .query = "default.schema()",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "age"}, {"type", "INT32"}},
       {{"fieldName", "country"}, {"type", "STRING"}},
       {{"fieldName", "date"}, {"type", "DATE32"}},
       {{"fieldName", "gene1"}, {"type", "STRING"}},
       {{"fieldName", "primaryKey"}, {"type", "STRING"}},
       {{"fieldName", "segment1"}, {"type", "STRING"}},
       {{"fieldName", "unaligned_segment1"}, {"type", "STRING"}}}
   )
};

// schema() after a pipeline reports the intermediate result's schema. The
// aggregate count column is INT64.
const QueryTestScenario GROUP_BY_SCHEMA_SCENARIO = {
   .name = "GROUP_BY_SCHEMA",
   .query = "default.filter(country='CH').groupBy({count := count()}, {age}).schema()",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "age"}, {"type", "INT32"}}, {{"fieldName", "count"}, {"type", "INT64"}}}
   )
};

// schema() output is itself a regular 2-column relation, so pipeline operators
// can be chained after it.
const QueryTestScenario CHAINING_SCHEMA_SCENARIO = {
   .name = "CHAINING_SCHEMA",
   .query =
      "default.filter(country='CH').groupBy({count := count()}, {age}).schema().project({type})",
   .expected_query_result = nlohmann::json({{{"type", "INT32"}}, {{"type", "INT64"}}})
};

// schema() on a mutations result reports the mutations default fields. The
// mutations `count` is INT32 (unlike the INT64 aggregate count). The queried
// sequence appears only via the derived columns, no NUCLEOTIDE_SEQUENCE column.
const QueryTestScenario MUTATIONS_SCHEMA_SCENARIO = {
   .name = "MUTATIONS_SCHEMA",
   .query = "default.mutations(minProportion:=0.1).schema()",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "mutation"}, {"type", "STRING"}},
       {{"fieldName", "mutationFrom"}, {"type", "STRING"}},
       {{"fieldName", "mutationTo"}, {"type", "STRING"}},
       {{"fieldName", "sequenceName"}, {"type", "STRING"}},
       {{"fieldName", "position"}, {"type", "INT32"}},
       {{"fieldName", "proportion"}, {"type", "FLOAT"}},
       {{"fieldName", "coverage"}, {"type", "INT32"}},
       {{"fieldName", "count"}, {"type", "INT32"}}}
   )
};

}  // namespace

QUERY_TEST(
   SchemaTest,
   TEST_DATA,
   ::testing::Values(
      TABLE_SCHEMA_SCENARIO,
      GROUP_BY_SCHEMA_SCENARIO,
      MUTATIONS_SCHEMA_SCENARIO,
      CHAINING_SCHEMA_SCENARIO
   )
);
