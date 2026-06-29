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
   bool is_covered,
   double proportion,
   const nlohmann::json& segment1 = nullptr
) {
   return {
      {"primaryKey", primaryKey},
      {"country", country},
      {"date", date},
      {"age", age},
      {"is_covered", is_covered},
      {"proportion", proportion},
      {"segment1", segment1},
      {"gene1", nullptr},
      {"unaligned_segment1", nullptr},
   };
}

nlohmann::json alignedSequence(const std::string& sequence) {
   return {{"sequence", sequence}, {"insertions", nlohmann::json::array()}};
}

const std::vector DATA = {
   createData("id_0", "CH", "2024-01-01", 31, true, 0.5, alignedSequence("ACGT")),
   createData("id_1", "DE", "2024-01-02", 42, false, 0.75, alignedSequence("ACGA")),
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
    - name: "is_covered"
      type: "boolean"
    - name: "proportion"
      type: "float"
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

const QueryTestScenario TABLE_SCHEMA_SCENARIO = {
   .name = "TABLE_SCHEMA",
   .query = "default.schema()",
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

const QueryTestScenario GROUP_BY_SCHEMA_SCENARIO = {
   .name = "GROUP_BY_SCHEMA",
   .query = "default.filter(country='CH').groupBy({count := count()}, {age}).schema()",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "age"}, {"type", "INT32"}}, {{"fieldName", "count"}, {"type", "INT64"}}}
   )
};

const QueryTestScenario CHAINING_SCHEMA_SCENARIO = {
   .name = "CHAINING_SCHEMA",
   .query =
      "default.filter(country='CH').groupBy({count := count()}, {age}).schema().project({type})",
   .expected_query_result = nlohmann::json({{{"type", "INT32"}}, {{"type", "INT64"}}})
};

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

const QueryTestScenario INSERTIONS_SCHEMA_SCENARIO = {
   .name = "INSERTIONS_SCHEMA",
   .query = "default.insertions().schema()",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "position"}, {"type", "INT32"}},
       {{"fieldName", "insertedSymbols"}, {"type", "STRING"}},
       {{"fieldName", "sequenceName"}, {"type", "STRING"}},
       {{"fieldName", "insertion"}, {"type", "STRING"}},
       {{"fieldName", "count"}, {"type", "INT32"}}}
   )
};

const QueryTestScenario SCHEMA_AFTER_MAP_SCENARIO = {
   .name = "SCHEMA_AFTER_MAP",
   .query = "default.map({tag := 42, label := 'x'}).project({primaryKey, tag, label}).schema()",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "primaryKey"}, {"type", "STRING"}},
       {{"fieldName", "tag"}, {"type", "INT64"}},
       {{"fieldName", "label"}, {"type", "STRING"}}}
   )
};

// project() before schema() controls field selection and order
const QueryTestScenario SCHEMA_AFTER_PROJECT_ORDER_SCENARIO = {
   .name = "SCHEMA_AFTER_PROJECT_ORDER",
   .query = "default.project({date, age, primaryKey}).schema()",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "date"}, {"type", "DATE32"}},
       {{"fieldName", "age"}, {"type", "INT32"}},
       {{"fieldName", "primaryKey"}, {"type", "STRING"}}}
   )
};

const QueryTestScenario MAP_AFTER_SCHEMA_SCENARIO = {
   .name = "MAP_AFTER_SCHEMA",
   .query = "default.groupBy({count := count()}, {age}).schema().map({kind := 'field'})",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "age"}, {"type", "INT32"}, {"kind", "field"}},
       {{"fieldName", "count"}, {"type", "INT64"}, {"kind", "field"}}}
   )
};

const QueryTestScenario SCHEMA_OF_SCHEMA_SCENARIO = {
   .name = "SCHEMA_OF_SCHEMA",
   .query = "default.schema().schema()",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "fieldName"}, {"type", "STRING"}}, {{"fieldName", "type"}, {"type", "STRING"}}
      }
   )
};

// Amino acid mutations resolve through a different node template than nucleotide mutations
const QueryTestScenario AMINO_ACID_MUTATIONS_SCHEMA_SCENARIO = {
   .name = "AMINO_ACID_MUTATIONS_SCHEMA",
   .query = "default.aminoAcidMutations(minProportion:=0.1).schema()",
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

// schema() inspects only the declared output schema, never the data: a child that
// filters out every row still reports the full table schema (not zero rows).
const QueryTestScenario SCHEMA_IGNORES_DATA_SCENARIO = {
   .name = "SCHEMA_IGNORES_DATA",
   .query = "default.filter(country='does-not-exist').project({primaryKey, age}).schema()",
   .expected_query_result = nlohmann::json(
      {{{"fieldName", "primaryKey"}, {"type", "STRING"}}, {{"fieldName", "age"}, {"type", "INT32"}}}
   )
};

// schema() takes no arguments beyond its input; passing an extra positional argument
// must be rejected at planning time.
const QueryTestScenario SCHEMA_EXTRA_ARG_ERROR_SCENARIO = {
   .name = "SCHEMA_EXTRA_ARG_ERROR",
   .query = "default.schema(age)",
   .expected_error_message = "schema() received too many positional arguments"
};

}  // namespace

QUERY_TEST(
   SchemaTest,
   TEST_DATA,
   ::testing::Values(
      TABLE_SCHEMA_SCENARIO,
      GROUP_BY_SCHEMA_SCENARIO,
      MUTATIONS_SCHEMA_SCENARIO,
      AMINO_ACID_MUTATIONS_SCHEMA_SCENARIO,
      INSERTIONS_SCHEMA_SCENARIO,
      SCHEMA_AFTER_MAP_SCENARIO,
      SCHEMA_AFTER_PROJECT_ORDER_SCENARIO,
      MAP_AFTER_SCHEMA_SCENARIO,
      SCHEMA_OF_SCHEMA_SCENARIO,
      SCHEMA_IGNORES_DATA_SCENARIO,
      SCHEMA_EXTRA_ARG_ERROR_SCENARIO,
      CHAINING_SCHEMA_SCENARIO
   )
);
