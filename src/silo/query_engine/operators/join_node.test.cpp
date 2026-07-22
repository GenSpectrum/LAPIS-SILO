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
      {"segment1", {{"sequence", "T"}, {"insertions", nlohmann::json::array()}}},
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

// Inner join on a unique key: every row matches exactly its renamed copy.
const QueryTestScenario JOIN_INNER_ON_KEY_SCENARIO = {
   .name = "JOIN_INNER_ON_KEY",
   .query = R"(join(
      default.project({primaryKey, country}),
      default.map({pk := primaryKey, ctry := country}).project({pk, ctry}),
      primaryKey = pk
   ).orderBy({asc(primaryKey)}))",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"country", "CH"}, {"pk", "id_0"}, {"ctry", "CH"}},
       {{"primaryKey", "id_1"}, {"country", "DE"}, {"pk", "id_1"}, {"ctry", "DE"}},
       {{"primaryKey", "id_2"}, {"country", "CH"}, {"pk", "id_2"}, {"ctry", "CH"}},
       {{"primaryKey", "id_3"}, {"country", "DE"}, {"pk", "id_3"}, {"ctry", "DE"}}}
   )
};

// Piped syntax: left.join(right, on) instead of join(left, right, on).
const QueryTestScenario JOIN_PIPED_SYNTAX_SCENARIO = {
   .name = "JOIN_PIPED_SYNTAX",
   .query = R"(
      default.project({primaryKey, country})
         .join(default.map({pk := primaryKey, ctry := country}).project({pk, ctry}), primaryKey = pk)
         .orderBy({asc(primaryKey)})
   )",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"country", "CH"}, {"pk", "id_0"}, {"ctry", "CH"}},
       {{"primaryKey", "id_1"}, {"country", "DE"}, {"pk", "id_1"}, {"ctry", "DE"}},
       {{"primaryKey", "id_2"}, {"country", "CH"}, {"pk", "id_2"}, {"ctry", "CH"}},
       {{"primaryKey", "id_3"}, {"country", "DE"}, {"pk", "id_3"}, {"ctry", "DE"}}}
   )
};

// Explicit `type := inner` with a filter on the left input.
const QueryTestScenario JOIN_EXPLICIT_INNER_SCENARIO = {
   .name = "JOIN_EXPLICIT_INNER",
   .query = R"(join(
      default.filter(country='CH').project({primaryKey, country}),
      default.map({pk := primaryKey, ctry := country}).project({pk, ctry}),
      primaryKey = pk,
      type := inner
   ).orderBy({asc(primaryKey)}))",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"country", "CH"}, {"pk", "id_0"}, {"ctry", "CH"}},
       {{"primaryKey", "id_2"}, {"country", "CH"}, {"pk", "id_2"}, {"ctry", "CH"}}}
   )
};

// Join on a non-key column yields the cross product of matching groups.
const QueryTestScenario JOIN_MANY_TO_MANY_SCENARIO = {
   .name = "JOIN_MANY_TO_MANY",
   .query = R"(join(
      default.filter(country='DE').project({primaryKey, country}),
      default.filter(country='DE').map({pk := primaryKey, ctry := country}).project({pk, ctry}),
      country = ctry
   ).orderBy({asc(primaryKey), asc(pk)}))",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_1"}, {"country", "DE"}, {"pk", "id_1"}, {"ctry", "DE"}},
       {{"primaryKey", "id_1"}, {"country", "DE"}, {"pk", "id_3"}, {"ctry", "DE"}},
       {{"primaryKey", "id_3"}, {"country", "DE"}, {"pk", "id_1"}, {"ctry", "DE"}},
       {{"primaryKey", "id_3"}, {"country", "DE"}, {"pk", "id_3"}, {"ctry", "DE"}}}
   )
};

// Left outer join: unmatched left rows keep null values for the right columns.
const QueryTestScenario JOIN_LEFT_OUTER_SCENARIO = {
   .name = "JOIN_LEFT_OUTER",
   .query = R"(join(
      default.project({primaryKey, country}),
      default.filter(country='CH').map({pk := primaryKey, ctry := country}).project({pk, ctry}),
      primaryKey = pk,
      type := left
   ).orderBy({asc(primaryKey)}))",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"country", "CH"}, {"pk", "id_0"}, {"ctry", "CH"}},
       {{"primaryKey", "id_1"}, {"country", "DE"}, {"pk", nullptr}, {"ctry", nullptr}},
       {{"primaryKey", "id_2"}, {"country", "CH"}, {"pk", "id_2"}, {"ctry", "CH"}},
       {{"primaryKey", "id_3"}, {"country", "DE"}, {"pk", nullptr}, {"ctry", nullptr}}}
   )
};

// Left semi join: keeps left rows that have a match, outputs only left columns.
const QueryTestScenario JOIN_LEFT_SEMI_SCENARIO = {
   .name = "JOIN_LEFT_SEMI",
   .query = R"(join(
      default.project({primaryKey, country}),
      default.filter(country='CH').map({pk := primaryKey, ctry := country}).project({pk, ctry}),
      primaryKey = pk,
      type := leftSemi
   ).orderBy({asc(primaryKey)}))",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"country", "CH"}}, {{"primaryKey", "id_2"}, {"country", "CH"}}}
   )
};

// Left anti join: keeps left rows WITHOUT a match, outputs only left columns.
const QueryTestScenario JOIN_LEFT_ANTI_SCENARIO = {
   .name = "JOIN_LEFT_ANTI",
   .query = R"(join(
      default.project({primaryKey, country}),
      default.filter(country='CH').map({pk := primaryKey, ctry := country}).project({pk, ctry}),
      primaryKey = pk,
      type := leftAnti
   ).orderBy({asc(primaryKey)}))",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_1"}, {"country", "DE"}}, {{"primaryKey", "id_3"}, {"country", "DE"}}}
   )
};

// A filter above the join over a left-only column is pushed into the left input.
const QueryTestScenario JOIN_DOWNSTREAM_FILTER_SCENARIO = {
   .name = "JOIN_DOWNSTREAM_FILTER",
   .query = R"(join(
      default.project({primaryKey, country}),
      default.map({pk := primaryKey, ctry := country}).project({pk, ctry}),
      primaryKey = pk
   ).filter(country='CH').orderBy({asc(primaryKey)}))",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"country", "CH"}, {"pk", "id_0"}, {"ctry", "CH"}},
       {{"primaryKey", "id_2"}, {"country", "CH"}, {"pk", "id_2"}, {"ctry", "CH"}}}
   )
};

// GroupBy applied to the join result.
const QueryTestScenario JOIN_WITH_GROUPBY_SCENARIO = {
   .name = "JOIN_WITH_GROUPBY",
   .query = R"(join(
      default.project({primaryKey, country}),
      default.map({pk := primaryKey, ctry := country}).project({pk, ctry}),
      primaryKey = pk
   ).groupBy({count := count()}, {country}).orderBy({asc(country)}))",
   .expected_query_result =
      nlohmann::json({{{"country", "CH"}, {"count", 2}}, {{"country", "DE"}, {"count", 2}}})
};

// Multiple equalities combined with `&&`.
const QueryTestScenario JOIN_MULTI_KEY_SCENARIO = {
   .name = "JOIN_MULTI_KEY",
   .query = R"(join(
      default.project({primaryKey, country}),
      default.map({pk := primaryKey, ctry := country}).project({pk, ctry}),
      primaryKey = pk && country = ctry
   ).orderBy({asc(primaryKey)}))",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"country", "CH"}, {"pk", "id_0"}, {"ctry", "CH"}},
       {{"primaryKey", "id_1"}, {"country", "DE"}, {"pk", "id_1"}, {"ctry", "DE"}},
       {{"primaryKey", "id_2"}, {"country", "CH"}, {"pk", "id_2"}, {"ctry", "CH"}},
       {{"primaryKey", "id_3"}, {"country", "DE"}, {"pk", "id_3"}, {"ctry", "DE"}}}
   )
};

// A column present in both inputs is ambiguous in the on-expression.
const QueryTestScenario JOIN_AMBIGUOUS_COLUMN_SCENARIO = {
   .name = "JOIN_AMBIGUOUS_COLUMN",
   .query = R"(join(
      default.project({primaryKey, country}),
      default.project({primaryKey, country}),
      primaryKey = primaryKey
   ))",
   .expected_query_result = {},
   .expected_error_message =
      "join() on-expression references column 'primaryKey', which exists in both inputs and is "
      "therefore ambiguous. Rename one side (e.g. via map()) before joining."
};

// An on-expression referencing a column that exists in neither input.
const QueryTestScenario JOIN_UNKNOWN_COLUMN_SCENARIO = {
   .name = "JOIN_UNKNOWN_COLUMN",
   .query = R"(join(
      default.project({primaryKey}),
      default.map({pk := primaryKey}).project({pk}),
      primaryKey = doesNotExist
   ))",
   .expected_query_result = {},
   .expected_error_message = "join() on-expression references unknown column 'doesNotExist'"
};

// An unknown join type symbol.
const QueryTestScenario JOIN_INVALID_TYPE_SCENARIO = {
   .name = "JOIN_INVALID_TYPE",
   .query = R"(join(
      default.project({primaryKey}),
      default.map({pk := primaryKey}).project({pk}),
      primaryKey = pk,
      type := sideways
   ))",
   .expected_query_result = {},
   .expected_error_message =
      "invalid join type 'sideways'. Valid types are: inner, left, right, full, leftSemi, "
      "rightSemi, leftAnti, rightAnti"
};
}  // namespace

QUERY_TEST(
   JoinTest,
   TEST_DATA,
   ::testing::Values(
      JOIN_INNER_ON_KEY_SCENARIO,
      JOIN_PIPED_SYNTAX_SCENARIO,
      JOIN_EXPLICIT_INNER_SCENARIO,
      JOIN_MANY_TO_MANY_SCENARIO,
      JOIN_LEFT_OUTER_SCENARIO,
      JOIN_LEFT_SEMI_SCENARIO,
      JOIN_LEFT_ANTI_SCENARIO,
      JOIN_DOWNSTREAM_FILTER_SCENARIO,
      JOIN_WITH_GROUPBY_SCENARIO,
      JOIN_MULTI_KEY_SCENARIO,
      JOIN_AMBIGUOUS_COLUMN_SCENARIO,
      JOIN_UNKNOWN_COLUMN_SCENARIO,
      JOIN_INVALID_TYPE_SCENARIO
   )
);
