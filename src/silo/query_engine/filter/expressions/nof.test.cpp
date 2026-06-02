#include "silo/query_engine/filter/expressions/nof.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/filter/expressions/and.h"
#include "silo/query_engine/filter/expressions/false.h"
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
   children.emplace_back(std::make_unique<False>());

   const NOf nof(std::move(children), 1, true);

   EXPECT_EQ(nof.toString(), "[exactly-1-of:True, False]");
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

TEST(NOfRewrite, shouldRewriteChildrenInNoneMode) {
   const silo::storage::Table table(std::make_shared<schema::TableSchema>());

   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<False>());

   const NOf nof(std::move(children), 1, false);

   auto rewritten = nof.rewrite(table, Expression::AmbiguityMode::NONE);

   EXPECT_EQ(rewritten->toString(), "[1-of:True, False]");
}

TEST(NOfRewrite, shouldPreserveExactMatchInNoneMode) {
   const silo::storage::Table table(std::make_shared<schema::TableSchema>());

   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<True>());

   const NOf nof(std::move(children), 2, true);

   auto rewritten = nof.rewrite(table, Expression::AmbiguityMode::NONE);

   EXPECT_EQ(rewritten->toString(), "[exactly-2-of:True, True, True]");
}

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

// --- compile tests (via empty table) ---

TEST(NOfCompile, shouldReturnFullWhenCountIsZeroAndNotExact) {
   const silo::storage::Table table(std::make_shared<schema::TableSchema>());

   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<True>());

   const NOf nof(std::move(children), 0, false);

   auto op = nof.compile(table);
   EXPECT_EQ(op->type(), operators::FULL);
}

TEST(NOfCompile, shouldReturnEmptyWhenCountExceedsChildren) {
   const silo::storage::Table table(std::make_shared<schema::TableSchema>());

   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());

   const NOf nof(std::move(children), 5, false);

   auto op = nof.compile(table);
   EXPECT_EQ(op->type(), operators::EMPTY);
}

TEST(NOfCompile, shouldReturnEmptyWhenExactCountExceedsChildren) {
   const silo::storage::Table table(std::make_shared<schema::TableSchema>());

   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());

   const NOf nof(std::move(children), 5, true);

   auto op = nof.compile(table);
   EXPECT_EQ(op->type(), operators::EMPTY);
}

TEST(NOfCompile, shouldHandleSingleChildAtLeastOne) {
   const silo::storage::Table table(std::make_shared<schema::TableSchema>());

   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());

   const NOf nof(std::move(children), 1, false);

   auto op = nof.compile(table);
   // Single True child with count=1 → Full operator (True compiles to Full, then count adjusted)
   EXPECT_EQ(op->type(), operators::FULL);
}

TEST(NOfCompile, shouldHandleAllFalseChildren) {
   const silo::storage::Table table(std::make_shared<schema::TableSchema>());

   ExpressionVector children;
   children.emplace_back(std::make_unique<False>());
   children.emplace_back(std::make_unique<False>());
   children.emplace_back(std::make_unique<False>());

   // All False children compile to Empty, which gets skipped in mapChildExpressions
   // updated_number_of_matchers stays 2, but no remaining children → count > children → Empty
   const NOf nof(std::move(children), 2, false);

   auto op = nof.compile(table);
   EXPECT_EQ(op->type(), operators::EMPTY);
}

TEST(NOfCompile, shouldHandleAllTrueChildren) {
   const silo::storage::Table table(std::make_shared<schema::TableSchema>());

   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<True>());

   // All True children compile to Full, each decrements count: 2 → -1
   // count < 0, not exact → Full
   const NOf nof(std::move(children), 2, false);

   auto op = nof.compile(table);
   EXPECT_EQ(op->type(), operators::FULL);
}

TEST(NOfCompile, shouldReturnEmptyForExactWithNegativeAdjustedCount) {
   const silo::storage::Table table(std::make_shared<schema::TableSchema>());

   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<True>());

   // All True → count goes 1 → -2 (negative). Exact + negative → Empty
   const NOf nof(std::move(children), 1, true);

   auto op = nof.compile(table);
   EXPECT_EQ(op->type(), operators::EMPTY);
}

TEST(NOfCompile, shouldReturnFullForExactZeroWithNoChildren) {
   const silo::storage::Table table(std::make_shared<schema::TableSchema>());

   ExpressionVector children;

   // exactly-0-of with 0 children → Full
   const NOf nof(std::move(children), 0, true);

   auto op = nof.compile(table);
   EXPECT_EQ(op->type(), operators::FULL);
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
      NOF_WITH_NEGATION
   )
);
