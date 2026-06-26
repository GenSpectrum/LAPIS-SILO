#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

nlohmann::json createData(
   const std::string& primaryKey,
   int value,
   const std::string& str_value,
   const nlohmann::json& segment1 = nullptr,
   const nlohmann::json& unaligned_segment1 = nullptr
) {
   return {
      {"primaryKey", primaryKey},
      {"int_value", value},
      {"str_value", str_value},
      {"segment1", segment1},
      {"gene1", nullptr},
      {"unaligned_segment1", unaligned_segment1}
   };
}

nlohmann::json alignedSequence(const std::string& sequence) {
   return {{"sequence", sequence}, {"insertions", nlohmann::json::array()}};
}

// id_0 carries an aligned sequence, id_1 has no sequence (null) so that `at` on a
// sequence column can be checked for both a present and an absent value. The
// str_value column holds strings of differing lengths so that `at` can be checked
// for a position that is out of bounds for one value but in bounds for the other.
// unaligned_segment1 carries the (zstd-compressed) unaligned sequence used by the
// decompression scenarios.
const std::vector<nlohmann::json> DATA = {
   createData("id_0", 1, "short", alignedSequence("ACGT"), "ACGT"),
   createData("id_1", 2, "longlonglong")
};

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
    - name: "str_value"
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

const QueryTestScenario MAP_OVERRIDE_TWICE_SCENARIO = {
   .name = "MAP_OVERRIDE_TWICE",
   .query = "default.map({int_value := 42}).map({int_value := 5}).project({primaryKey, int_value})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"int_value", 5}}, {{"primaryKey", "id_1"}, {"int_value", 5}}}
   )
};

const QueryTestScenario MAP_INT64_SCENARIO = {
   .name = "MAP_INT64",
   .query = "default.map({x := 3000000000}).project({primaryKey, x})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"x", 3000000000}}, {{"primaryKey", "id_1"}, {"x", 3000000000}}}
   )
};

// A column reference assigns an existing column's value to a new column.
const QueryTestScenario MAP_FIELD_REF_SCENARIO = {
   .name = "MAP_FIELD_REF",
   .query = "default.map({copied := int_value}).project({primaryKey, copied})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"copied", 1}}, {{"primaryKey", "id_1"}, {"copied", 2}}}
   )
};

// `at` extracts the (1-indexed) character of a string column at a position.
const QueryTestScenario MAP_AT_SCENARIO = {
   .name = "MAP_AT",
   .query = "default.map({second := primaryKey.at(4)}).project({primaryKey, second})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"second", "0"}}, {{"primaryKey", "id_1"}, {"second", "1"}}}
   )
};

// When the position is past the end of the string, `at` yields an empty string
// rather than failing. id_0's str_value "short" has only 5 characters, so at(8) is
// out of bounds and produces ""; id_1's "longlonglong" has a character at 8 ("g").
const QueryTestScenario MAP_AT_OUT_OF_BOUNDS_SCENARIO = {
   .name = "MAP_AT_OUT_OF_BOUNDS",
   .query = "default.map({eighth := str_value.at(8)}).project({primaryKey, eighth})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"eighth", ""}}, {{"primaryKey", "id_1"}, {"eighth", "g"}}}
   )
};

// `at` works on a (zstd-compressed) nucleotide sequence column: the sequence is
// decompressed and the (1-indexed) character at the position is extracted. id_0's
// aligned sequence is "ACGT"; id_1 has no sequence, so the value is null.
const QueryTestScenario MAP_AT_SEQUENCE_FIRST_SCENARIO = {
   .name = "MAP_AT_SEQUENCE_FIRST",
   .query = "default.map({base := segment1.at(1)}).project({primaryKey, base})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"base", "A"}}, {{"primaryKey", "id_1"}, {"base", nullptr}}}
   )
};

const QueryTestScenario MAP_AT_SEQUENCE_INNER_SCENARIO = {
   .name = "MAP_AT_SEQUENCE_INNER",
   .query = "default.map({base := segment1.at(3)}).project({primaryKey, base})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"base", "G"}}, {{"primaryKey", "id_1"}, {"base", nullptr}}}
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

// A map() that assigns the same output column twice is rejected at query construction
// time rather than silently dropping one assignment.
const QueryTestScenario MAP_DUPLICATE_OUTPUT_NAME_SCENARIO = {
   .name = "MAP_DUPLICATE_OUTPUT_NAME",
   .query = "default.map({x := 1, x := 2}).project({primaryKey, x})",
   .expected_query_result = {},
   .expected_error_message = "map() assigns the output column 'x' more than once"
};

// A table scan that exposes a (compressed) sequence column is wrapped in a MapNode
// holding a ZstdDecompressScalar assignment by wrapWithDecompressIfNeeded. This
// exercises the decompression path (backpressure sink/source + ProjectNode) of
// MapNode::addToExecPlan end-to-end.
const QueryTestScenario DECOMPRESS_SEQUENCE_SCENARIO = {
   .name = "DECOMPRESS_SEQUENCE",
   .query = "default.project({primaryKey, unaligned_segment1}).orderBy({primaryKey})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"unaligned_segment1", "ACGT"}},
       {{"primaryKey", "id_1"}, {"unaligned_segment1", nullptr}}}
   )
};

// A user map() stacked on top of the implicit decompression MapNode: both the
// decompressed sequence column and a mapped literal column flow through.
const QueryTestScenario DECOMPRESS_WITH_USER_MAP_SCENARIO = {
   .name = "DECOMPRESS_WITH_USER_MAP",
   .query =
      "default.map({tag := 7}).project({primaryKey, unaligned_segment1, "
      "tag}).orderBy({primaryKey})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"unaligned_segment1", "ACGT"}, {"tag", 7}},
       {{"primaryKey", "id_1"}, {"unaligned_segment1", nullptr}, {"tag", 7}}}
   )
};

}  // namespace

QUERY_TEST(
   MapTest,
   TEST_DATA,
   ::testing::Values(
      MAP_LITERALS_SCENARIO,
      MAP_OVERRIDE_SCENARIO,
      MAP_OVERRIDE_TWICE_SCENARIO,
      MAP_INT64_SCENARIO,
      MAP_FIELD_REF_SCENARIO,
      MAP_AT_SCENARIO,
      MAP_AT_OUT_OF_BOUNDS_SCENARIO,
      MAP_AT_SEQUENCE_FIRST_SCENARIO,
      MAP_AT_SEQUENCE_INNER_SCENARIO,
      MAP_ONLY_MAPPED_COLUMN_SCENARIO,
      MAP_DUPLICATE_OUTPUT_NAME_SCENARIO,
      DECOMPRESS_SEQUENCE_SCENARIO,
      DECOMPRESS_WITH_USER_MAP_SCENARIO
   )
);
