#include "silo/query_engine/filter/expressions/nof.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/filter/expressions/and.h"
#include "silo/query_engine/filter/expressions/negation.h"
#include "silo/query_engine/filter/expressions/true.h"
#include "silo/test/query_fixture.test.h"

namespace silo::query_engine::filter::expressions {

// --- toString tests ---

TEST(NOfToString, shouldFormatAtLeastN) {
   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<True>());

   const NOf nof(std::move(children), 2, false);

   EXPECT_EQ(nof.toString(), "[2-of:True, True, True]");
}

TEST(NOfToString, shouldFormatExactlyN) {
   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<True>());

   const NOf nof(std::move(children), 1, true);

   EXPECT_EQ(nof.toString(), "[exactly-1-of:True, True]");
}

TEST(NOfToString, shouldHandleSingleChild) {
   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());

   const NOf nof(std::move(children), 1, false);

   EXPECT_EQ(nof.toString(), "[1-of:True]");
}

TEST(NOfToString, shouldHandleEmptyChildren) {
   ExpressionVector children;

   const NOf nof(std::move(children), 0, false);

   EXPECT_EQ(nof.toString(), "[0-of:]");
}

// --- rewrite tests ---

TEST(NOfRewrite, shouldDecomposeExactMatchInAmbiguityMode) {
   const silo::storage::Table table(std::make_shared<schema::TableSchema>());

   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<True>());

   const NOf nof(std::move(children), 2, true);

   auto rewritten = nof.rewrite(table, Expression::AmbiguityMode::UPPER_BOUND);

   // exact-2-of with ambiguity rewrites to: And(NOf(2,false), !NOf(3,false))
   EXPECT_EQ(
      rewritten->toString(), "And([2-of:True, True, True] & !([3-of:True, True, True]))"
   );
}

TEST(NOfRewrite, shouldNotDecomposeExactWhenNEqualsChildCount) {
   const silo::storage::Table table(std::make_shared<schema::TableSchema>());

   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<True>());

   // exactly-2-of with 2 children: number_of_matchers == children.size(), no decompose
   const NOf nof(std::move(children), 2, true);

   auto rewritten = nof.rewrite(table, Expression::AmbiguityMode::UPPER_BOUND);

   EXPECT_EQ(rewritten->toString(), "[exactly-2-of:True, True]");
}

}  // namespace silo::query_engine::filter::expressions

// --- Query integration tests ---
namespace {

using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

nlohmann::json createNOfData(
   const std::string& primary_key,
   const std::string& country,
   const std::string& region,
   const std::string& date
) {
   return nlohmann::json::parse(fmt::format(
      R"(
{{
   "primaryKey": "{}",
   "country": "{}",
   "region": "{}",
   "date": "{}",
   "segment1": {{
      "sequence": "ACGT",
      "insertions": []
   }}
}}
)",
      primary_key,
      country,
      region,
      date
   ));
}

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
     generateIndex: true
   - name: "region"
     type: "string"
     generateIndex: true
   - name: "date"
     type: "date"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "ATGCN"}},
   {},
};

// Data:
// id_0: Switzerland, Europe, 2020-01-01  → matches country=Switzerland, region=Europe
// id_1: Germany, Europe, 2021-06-15      → matches country=Germany, region=Europe
// id_2: USA, Americas, 2019-03-20        → matches country=USA, region=Americas
// id_3: Switzerland, Europe, 2018-11-01  → matches country=Switzerland, region=Europe
// id_4: France, Europe, 2022-07-10       → matches country=France, region=Europe
const QueryTestData NOF_TEST_DATA{
   .ndjson_input_data =
      {createNOfData("id_0", "Switzerland", "Europe", "2020-01-01"),
       createNOfData("id_1", "Germany", "Europe", "2021-06-15"),
       createNOfData("id_2", "USA", "Americas", "2019-03-20"),
       createNOfData("id_3", "Switzerland", "Europe", "2018-11-01"),
       createNOfData("id_4", "France", "Europe", "2022-07-10")},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .without_unaligned_sequences = true
};

// nOf(2, {country='Switzerland', country='Germany', region='Europe'})
// Switzerland+Europe: id_0, id_3 match 2 of 3
// Germany+Europe: id_1 matches 2 of 3
// USA+Americas: id_2 matches 0 of 3
// France+Europe: id_4 matches 1 of 3 (region=Europe only)
// → at least 2: id_0, id_1, id_3
const QueryTestScenario NOF_AT_LEAST_2_OF_3 = {
   .name = "NOF_AT_LEAST_2_OF_3",
   .query =
      "default.filter(nOf(2, {country = 'Switzerland', country = 'Germany', region = "
      "'Europe'})).project({primaryKey, country})",
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"Switzerland","primaryKey":"id_0"},
{"country":"Germany","primaryKey":"id_1"},
{"country":"Switzerland","primaryKey":"id_3"}
])"
   )
};

// nOf(3, {country='Switzerland', country='Germany', region='Europe'})
// Need all 3 → impossible (can't be both Switzerland AND Germany)
// → empty
const QueryTestScenario NOF_AT_LEAST_3_OF_3 = {
   .name = "NOF_AT_LEAST_3_OF_3",
   .query =
      "default.filter(nOf(3, {country = 'Switzerland', country = 'Germany', region = "
      "'Europe'})).project({primaryKey})",
   .expected_query_result = nlohmann::json::parse(R"([])")
};

// nOf(1, {country='Switzerland', country='USA'})
// → at least 1: id_0, id_2, id_3
const QueryTestScenario NOF_AT_LEAST_1_OF_2 = {
   .name = "NOF_AT_LEAST_1_OF_2",
   .query =
      "default.filter(nOf(1, {country = 'Switzerland', country = 'USA'})).project({primaryKey, "
      "country})",
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"Switzerland","primaryKey":"id_0"},
{"country":"USA","primaryKey":"id_2"},
{"country":"Switzerland","primaryKey":"id_3"}
])"
   )
};

// nOf(1, {country='Switzerland', region='Europe'}, matchExactly:=true)
// exactly 1 match:
// id_0: Switzerland+Europe → 2 matches → no
// id_1: Germany+Europe → 1 match (region) → yes
// id_2: USA+Americas → 0 → no
// id_3: Switzerland+Europe → 2 matches → no
// id_4: France+Europe → 1 match (region) → yes
const QueryTestScenario NOF_EXACTLY_1_OF_2 = {
   .name = "NOF_EXACTLY_1_OF_2",
   .query =
      "default.filter(nOf(1, {country = 'Switzerland', region = 'Europe'}, "
      "matchExactly:=true)).project({primaryKey, country, region})",
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"Germany","primaryKey":"id_1","region":"Europe"},
{"country":"France","primaryKey":"id_4","region":"Europe"}
])"
   )
};

// nOf(2, {country='Switzerland', region='Europe'}, matchExactly:=true)
// exactly 2 → both must match
// id_0: Switzerland+Europe → 2 → yes
// id_3: Switzerland+Europe → 2 → yes
const QueryTestScenario NOF_EXACTLY_2_OF_2 = {
   .name = "NOF_EXACTLY_2_OF_2",
   .query =
      "default.filter(nOf(2, {country = 'Switzerland', region = 'Europe'}, "
      "matchExactly:=true)).project({primaryKey, country})",
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"Switzerland","primaryKey":"id_0"},
{"country":"Switzerland","primaryKey":"id_3"}
])"
   )
};

// nOf(0, {country='Switzerland', country='Germany'})
// at least 0 → everything matches
const QueryTestScenario NOF_AT_LEAST_0 = {
   .name = "NOF_AT_LEAST_0",
   .query =
      "default.filter(nOf(0, {country = 'Switzerland', country = "
      "'Germany'})).project({primaryKey})",
   .expected_query_result = nlohmann::json::parse(
      R"([
{"primaryKey":"id_0"},
{"primaryKey":"id_1"},
{"primaryKey":"id_2"},
{"primaryKey":"id_3"},
{"primaryKey":"id_4"}
])"
   )
};

// nOf(0, {country='Switzerland', region='Europe'}, matchExactly:=true)
// exactly 0 → neither matches
// id_2: USA+Americas → 0 matches → yes
const QueryTestScenario NOF_EXACTLY_0 = {
   .name = "NOF_EXACTLY_0",
   .query =
      "default.filter(nOf(0, {country = 'Switzerland', region = 'Europe'}, "
      "matchExactly:=true)).project({primaryKey, country})",
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"USA","primaryKey":"id_2"}
])"
   )
};

// nOf with negation child: nOf(2, {country='Switzerland', !country='USA'})
// child 1: country=Switzerland → id_0, id_3
// child 2: NOT country=USA → id_0, id_1, id_3, id_4
// at least 2: id_0, id_3 (match both)
const QueryTestScenario NOF_WITH_NEGATION = {
   .name = "NOF_WITH_NEGATION",
   .query =
      "default.filter(nOf(2, {country = 'Switzerland', !(country = "
      "'USA')})).project({primaryKey, country})",
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"Switzerland","primaryKey":"id_0"},
{"country":"Switzerland","primaryKey":"id_3"}
])"
   )
};

// --- Gap coverage: handleOrCase with negated child (DeMorgan path, L109-113) ---
// nOf(1, {country='Switzerland', !(country='Germany')})
// count=1, not exact, 2 children (1 non-negated, 1 negated) → handleOrCase with negated
// child 1: country=Switzerland → id_0, id_3
// child 2: NOT country=Germany → id_0, id_2, id_3, id_4
// at least 1: OR via DeMorgan → id_0, id_2, id_3, id_4
const QueryTestScenario NOF_OR_WITH_NEGATED_CHILD = {
   .name = "NOF_OR_WITH_NEGATED_CHILD",
   .query =
      "default.filter(nOf(1, {country = 'Switzerland', !(country = "
      "'Germany')})).project({primaryKey, country})",
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"Switzerland","primaryKey":"id_0"},
{"country":"USA","primaryKey":"id_2"},
{"country":"Switzerland","primaryKey":"id_3"},
{"country":"France","primaryKey":"id_4"}
])"
   )
};

// --- Gap coverage: exactly-0-of with 1 non-negated child (L62-64, Complement) ---
// nOf(0, {country='Switzerland'}, matchExactly:=true)
// count=0, exact, 1 non-negated child → Complement(IndexScan)
// Matches everything except Switzerland: id_1, id_2, id_4
const QueryTestScenario NOF_EXACTLY_0_SINGLE_NON_NEGATED = {
   .name = "NOF_EXACTLY_0_SINGLE_NON_NEGATED",
   .query =
      "default.filter(nOf(0, {country = 'Switzerland'}, "
      "matchExactly:=true)).project({primaryKey, country})",
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"Germany","primaryKey":"id_1"},
{"country":"USA","primaryKey":"id_2"},
{"country":"France","primaryKey":"id_4"}
])"
   )
};

// --- Gap coverage: exactly-0-of with 1 negated child (L59-60, return negated) ---
// nOf(0, {!(country='Switzerland')}, matchExactly:=true)
// count=0, exact, 1 negated child → return the inner (un-negated) operator directly
// NOT country=Switzerland compiles to Complement → mapChildExpressions negates it back →
// negated bucket has the raw IndexScan for country=Switzerland
// exactly-0 with 1 negated child means: the negated child must NOT match
// The negated child is "NOT Switzerland", so "NOT Switzerland must not match" = must be Switzerland
// Result: id_0, id_3
const QueryTestScenario NOF_EXACTLY_0_SINGLE_NEGATED = {
   .name = "NOF_EXACTLY_0_SINGLE_NEGATED",
   .query =
      "default.filter(nOf(0, {!(country = 'Switzerland')}, "
      "matchExactly:=true)).project({primaryKey, country})",
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"Switzerland","primaryKey":"id_0"},
{"country":"Switzerland","primaryKey":"id_3"}
])"
   )
};

// --- Gap coverage: handleAndCase all negated (L91-94, Complement(Union)) ---
// nOf(2, {!(country='Switzerland'), !(country='USA')})
// Both children are negated → mapChildExpressions puts both in negated bucket
// count=2 == child_count=2 → handleAndCase, non_negated empty → Complement(Union(negated))
// child 1: NOT Switzerland → negated → IndexScan(Switzerland)
// child 2: NOT USA → negated → IndexScan(USA)
// handleAndCase: Complement(Union(Switzerland, USA)) = NOT(Switzerland OR USA)
// → matches: id_1 (Germany), id_4 (France)
const QueryTestScenario NOF_ALL_NEGATED_AND_CASE = {
   .name = "NOF_ALL_NEGATED_AND_CASE",
   .query =
      "default.filter(nOf(2, {!(country = 'Switzerland'), !(country = "
      "'USA')})).project({primaryKey, country})",
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"Germany","primaryKey":"id_1"},
{"country":"France","primaryKey":"id_4"}
])"
   )
};

// --- Gap coverage: exactly-0-of mixed negated/non-negated multi (L73-75) ---
// nOf(0, {country='Switzerland', !(country='USA')}, matchExactly:=true)
// count=0, exact, 2 children (1 non-negated, 1 negated), negated not empty
// → Intersection(negated, non_negated) where negated = [IndexScan(USA)],
//   non_negated = [IndexScan(Switzerland)]
// This means: Intersection with negated = "not USA" applied to "not Switzerland"
// Wait — for exactly-0, we need NONE to match. The code builds
// Intersection(negated_ops=[USA_scan], non_negated_ops=[Switzerland_scan])
// Intersection treats negated as: result AND NOT(negated)
// So: Switzerland AND NOT(USA) — but that's wrong for "exactly 0 match"
// Actually: exactly-0 means neither child matches. Inversion of Union.
// L73-75: Intersection(negated_child_operators, non_negated_child_operators)
//   non_negated becomes the "negated" arg of Intersection (to be complemented)
//   negated becomes the "non_negated" arg of Intersection (already flipped)
// So: negated_ops (USA scan) as non-neg, non_neg_ops (Switzerland scan) as neg
// = USA_scan AND NOT Switzerland_scan = is USA and not Switzerland
// That means: rows matching USA but not Switzerland → id_2
// But we want "neither Switzerland NOR NOT-USA matches"
// "country=Switzerland doesn't match" AND "NOT country=USA doesn't match"
// = NOT Switzerland AND NOT(NOT USA) = NOT Switzerland AND USA = USA AND NOT Switzerland
// id_2 is USA and not Switzerland → yes
// So result = [id_2]
const QueryTestScenario NOF_EXACTLY_0_MIXED_NEGATED = {
   .name = "NOF_EXACTLY_0_MIXED_NEGATED",
   .query =
      "default.filter(nOf(0, {country = 'Switzerland', !(country = 'USA')}, "
      "matchExactly:=true)).project({primaryKey, country})",
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"USA","primaryKey":"id_2"}
])"
   )
};

// --- Gap coverage: exact count exceeds number of children → empty result ---
// nOf(5, {country='Switzerland', country='USA'}, matchExactly:=true)
// count=5 but only 2 children → impossible → empty
const QueryTestScenario NOF_EXACT_COUNT_EXCEEDS_CHILDREN = {
   .name = "NOF_EXACT_COUNT_EXCEEDS_CHILDREN",
   .query =
      "default.filter(nOf(5, {country = 'Switzerland', country = 'USA'}, "
      "matchExactly:=true)).project({primaryKey})",
   .expected_query_result = nlohmann::json::parse(R"([])")
};

// --- Gap coverage: single negated child, count=1 (L80-81, Complement) ---
// nOf(1, {!(country='Switzerland')})
// count=1, 1 child (negated) → Complement of IndexScan(Switzerland)
// Matches everything except Switzerland: id_1, id_2, id_4
const QueryTestScenario NOF_SINGLE_NEGATED_CHILD = {
   .name = "NOF_SINGLE_NEGATED_CHILD",
   .query =
      "default.filter(nOf(1, {!(country = 'Switzerland')})).project({primaryKey, country})",
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"Germany","primaryKey":"id_1"},
{"country":"USA","primaryKey":"id_2"},
{"country":"France","primaryKey":"id_4"}
])"
   )
};

// --- Gap coverage: exactly-1-of with 3 children → Threshold exact path (L143-149) ---
// nOf(1, {country='Switzerland', country='Germany', region='Europe'}, matchExactly:=true)
// count=1, exact, 3 non-trivial children → not handled by trivial/and/or cases → Threshold
// id_0: Switzerland=yes, Germany=no, Europe=yes → 2 matches → no
// id_1: Switzerland=no, Germany=yes, Europe=yes → 2 matches → no
// id_2: Switzerland=no, Germany=no, Europe=no → 0 matches → no
// id_3: Switzerland=yes, Germany=no, Europe=yes → 2 matches → no
// id_4: Switzerland=no, Germany=no, Europe=yes → 1 match → yes
const QueryTestScenario NOF_EXACTLY_1_OF_3_THRESHOLD = {
   .name = "NOF_EXACTLY_1_OF_3_THRESHOLD",
   .query =
      "default.filter(nOf(1, {country = 'Switzerland', country = 'Germany', region = 'Europe'}, "
      "matchExactly:=true)).project({primaryKey, country})",
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"France","primaryKey":"id_4"}
])"
   )
};

}  // namespace

QUERY_TEST(
   NOf,
   NOF_TEST_DATA,
   ::testing::Values(
      NOF_AT_LEAST_2_OF_3,
      NOF_AT_LEAST_3_OF_3,
      NOF_AT_LEAST_1_OF_2,
      NOF_EXACTLY_1_OF_2,
      NOF_EXACTLY_2_OF_2,
      NOF_AT_LEAST_0,
      NOF_EXACTLY_0,
      NOF_WITH_NEGATION,
      NOF_OR_WITH_NEGATED_CHILD,
      NOF_EXACTLY_0_SINGLE_NON_NEGATED,
      NOF_EXACTLY_0_SINGLE_NEGATED,
      NOF_ALL_NEGATED_AND_CASE,
      NOF_EXACTLY_0_MIXED_NEGATED,
      NOF_EXACT_COUNT_EXCEEDS_CHILDREN,
      NOF_SINGLE_NEGATED_CHILD,
      NOF_EXACTLY_1_OF_3_THRESHOLD
   )
);
