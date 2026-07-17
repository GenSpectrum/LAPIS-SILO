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
   const nlohmann::json& unaligned_segment1 = nullptr,
   const std::string& date_value = ""
) {
   return {
      {"primaryKey", primaryKey},
      {"int_value", value},
      {"str_value", str_value},
      {"segment1", segment1},
      {"gene1", nullptr},
      {"unaligned_segment1", unaligned_segment1},
      {"date", date_value.empty() ? nlohmann::json(nullptr) : nlohmann::json(date_value)}
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
   createData("id_0", 1, "short", alignedSequence("ACGT"), "ACGT", "2023-01-05"),
   createData("id_1", 2, "longlonglong", nullptr, nullptr, "2023-12-31")
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

// Regression guard: selecting the (zstd-compressed) sequence column together with a
// `limit` must still return the correct, order-stable result.
const QueryTestScenario DECOMPRESS_SEQUENCE_WITH_LIMIT_SCENARIO = {
   .name = "DECOMPRESS_SEQUENCE_WITH_LIMIT",
   .query = "default.project({primaryKey, unaligned_segment1}).orderBy({primaryKey}).limit(1)",
   .expected_query_result =
      nlohmann::json({{{"primaryKey", "id_0"}, {"unaligned_segment1", "ACGT"}}})
};

// Correctness regression guard for a shape MapPullupPass is eligible to rewrite:
// `map({...}).limit(...)` builds a FetchNode directly above the user MapNode
// (Fetch(Map(scan))), which the pass may rewrite to Map(Fetch(scan)). QUERY_TEST asserts
// only the query result (it does not inspect the optimized plan), so this checks that the
// rewrite - when it happens - does not change the result.
const QueryTestScenario MAP_WITH_LIMIT_TRIGGERS_PULLUP_SCENARIO = {
   .name = "MAP_WITH_LIMIT_TRIGGERS_PULLUP",
   .query = "default.map({a := 3}).orderBy({primaryKey}).map({b := 7}).limit(1).project({a, b})",
   .expected_query_result = nlohmann::json({{{"a", 3}, {"b", 7}}})
};

// --- map()/filter() ordering combinations ---
//
// FilterPushdownPass keeps a MapNode on top of a filter and pushes the filter down into the
// TableScan (effectively swapping Filter(Map)->Map(Filter)). These scenarios assert that both
// orderings produce the correct result (a filter that references a passed-through column, never
// a Map-produced one).

// filter() stacked on top of map(): Filter(Map(scan)). FilterPushdownPass keeps the Map on top
// and pushes the filter (on the passed-through `int_value`) down into the scan.
const QueryTestScenario FILTER_ON_TOP_OF_MAP_SCENARIO = {
   .name = "FILTER_ON_TOP_OF_MAP",
   .query = "default.map({a := 3}).filter(int_value = 1).project({primaryKey, a})",
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_0"}, {"a", 3}}})
};

// map() stacked on top of filter(): Map(Filter(scan)). The Map is already above the filter,
// so nothing is swapped; the filter is pushed to the scan.
const QueryTestScenario MAP_ON_TOP_OF_FILTER_SCENARIO = {
   .name = "MAP_ON_TOP_OF_FILTER",
   .query = "default.filter(int_value = 1).map({a := 3}).project({primaryKey, a})",
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_0"}, {"a", 3}}})
};

// A map() whose assignment reads a passed-through column, stacked below a filter on a
// different passed-through column.
const QueryTestScenario FILTER_ON_TOP_OF_MAP_FIELD_REF_SCENARIO = {
   .name = "FILTER_ON_TOP_OF_MAP_FIELD_REF",
   .query =
      "default.map({copied := int_value}).filter(str_value = 'short').project({primaryKey, "
      "copied})",
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_0"}, {"copied", 1}}})
};

// filter() over the implicit decompression MapNode: Filter(Map_decompress(scan)). The filter is
// pushed below the decompression Map into the scan, so only matching rows are decompressed.
const QueryTestScenario FILTER_OVER_DECOMPRESS_MAP_SCENARIO = {
   .name = "FILTER_OVER_DECOMPRESS_MAP",
   .query = "default.filter(int_value = 1).project({primaryKey, unaligned_segment1})",
   .expected_query_result =
      nlohmann::json({{{"primaryKey", "id_0"}, {"unaligned_segment1", "ACGT"}}})
};

// filter() + user map() + implicit decompression map() + limit, all stacked. The filter is
// pushed below the Maps into the scan, and MapPullupPass then pulls the Maps above the fetch.
const QueryTestScenario FILTER_MAP_DECOMPRESS_LIMIT_SCENARIO = {
   .name = "FILTER_MAP_DECOMPRESS_LIMIT",
   .query =
      "default.filter(int_value >= 1).map({tag := 7}).project({primaryKey, unaligned_segment1, "
      "tag}).orderBy({primaryKey}).limit(1)",
   .expected_query_result =
      nlohmann::json({{{"primaryKey", "id_0"}, {"unaligned_segment1", "ACGT"}, {"tag", 7}}})
};

// A filter that references a column the map produces (`a`) is not yet supported
const QueryTestScenario FILTER_ON_MAPPED_COLUMN_SCENARIO = {
   .name = "FILTER_ON_MAPPED_COLUMN",
   .query = "default.map({a := 3}).filter(a = 3).project({primaryKey})",
   .expected_query_result = {},
   .expected_error_message = "The database does not contain the column 'a'"
};

// `isoWeek` maps a date column to its ISO 8601 week number as an integer.
const QueryTestScenario MAP_ISO_WEEK_SCENARIO = {
   .name = "MAP_ISO_WEEK",
   .query = "default.map({week := date.isoWeek()}).project({primaryKey, week})",
   .expected_query_result =
      nlohmann::json({{{"primaryKey", "id_0"}, {"week", 1}}, {{"primaryKey", "id_1"}, {"week", 52}}}
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
      DECOMPRESS_WITH_USER_MAP_SCENARIO,
      DECOMPRESS_SEQUENCE_WITH_LIMIT_SCENARIO,
      MAP_WITH_LIMIT_TRIGGERS_PULLUP_SCENARIO,
      FILTER_ON_TOP_OF_MAP_SCENARIO,
      MAP_ON_TOP_OF_FILTER_SCENARIO,
      FILTER_ON_TOP_OF_MAP_FIELD_REF_SCENARIO,
      FILTER_OVER_DECOMPRESS_MAP_SCENARIO,
      FILTER_MAP_DECOMPRESS_LIMIT_SCENARIO,
      FILTER_ON_MAPPED_COLUMN_SCENARIO,
      MAP_ISO_WEEK_SCENARIO
   )
);

namespace {

const std::vector<nlohmann::json> ISO_WEEK_NULL_DATA = {
   createData("id_0", 1, "short"),
   createData("id_1", 2, "short", nullptr, nullptr, "2020-12-31")
};

const QueryTestData ISO_WEEK_NULL_TEST_DATA{
   .ndjson_input_data = ISO_WEEK_NULL_DATA,
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

const QueryTestScenario MAP_ISO_WEEK_NULL_SCENARIO = {
   .name = "MAP_ISO_WEEK_NULL",
   .query = "default.map({week := date.isoWeek()}).project({primaryKey, week})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"week", nullptr}}, {{"primaryKey", "id_1"}, {"week", 53}}}
   )
};

}  // namespace

QUERY_TEST(
   MapIsoWeekNullTest,
   ISO_WEEK_NULL_TEST_DATA,
   ::testing::Values(MAP_ISO_WEEK_NULL_SCENARIO)
);
