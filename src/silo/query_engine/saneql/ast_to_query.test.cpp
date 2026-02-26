#include "silo/query_engine/saneql/ast_to_query.h"

#include <gtest/gtest.h>

#include "silo/query_engine/saneql/parse_exception.h"
#include "silo/query_engine/saneql/parser.h"

using silo::query_engine::saneql::convertToFilter;
using silo::query_engine::saneql::convertToQuery;
using silo::query_engine::saneql::ParseException;
using silo::query_engine::saneql::Parser;

namespace {

auto parseAndConvert(const std::string& input) {
   Parser parser(input);
   auto ast = parser.parse();
   return convertToQuery(*ast);
}

auto parseFilter(const std::string& input) {
   Parser parser(input);
   auto ast = parser.parse();
   return convertToFilter(*ast);
}

}  // namespace

// --- Equals ---

TEST(SaneQLAstToQuery, convertsStringEquals) {
   auto query = parseAndConvert("metadata.filter(country = 'USA').aggregated()");
   ASSERT_NE(query, nullptr);
   EXPECT_NE(query->filter, nullptr);
   EXPECT_NE(query->action, nullptr);
}

TEST(SaneQLAstToQuery, convertsIntEquals) {
   auto query = parseAndConvert("metadata.filter(age = 42).aggregated()");
   ASSERT_NE(query, nullptr);
   EXPECT_NE(query->filter, nullptr);
}

TEST(SaneQLAstToQuery, convertsFloatEquals) {
   auto query = parseAndConvert("metadata.filter(score = 3.14).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsBoolEquals) {
   auto query = parseAndConvert("metadata.filter(is_complete = true).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsNullEquals) {
   auto query = parseAndConvert("metadata.filter(host = null).aggregated()");
   ASSERT_NE(query, nullptr);
}

// --- NotEquals (<>) ---

TEST(SaneQLAstToQuery, convertsStringNotEquals) {
   auto query = parseAndConvert("metadata.filter(country <> 'USA').aggregated()");
   ASSERT_NE(query, nullptr);
   EXPECT_NE(query->filter, nullptr);
}

TEST(SaneQLAstToQuery, convertsIntNotEquals) {
   auto query = parseAndConvert("metadata.filter(age <> 42).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsFloatNotEquals) {
   auto query = parseAndConvert("metadata.filter(score <> 3.14).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsBoolNotEquals) {
   auto query = parseAndConvert("metadata.filter(is_complete <> true).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsNullNotEquals) {
   auto query = parseAndConvert("metadata.filter(host <> null).aggregated()");
   ASSERT_NE(query, nullptr);
}

// --- Comparison operators (<, <=, >, >=) ---

TEST(SaneQLAstToQuery, convertsIntLessThan) {
   auto query = parseAndConvert("metadata.filter(age < 30).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsIntLessEqual) {
   auto query = parseAndConvert("metadata.filter(age <= 30).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsIntGreaterThan) {
   auto query = parseAndConvert("metadata.filter(age > 30).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsIntGreaterEqual) {
   auto query = parseAndConvert("metadata.filter(age >= 30).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsFloatLessThan) {
   auto query = parseAndConvert("metadata.filter(score < 0.5).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsFloatGreaterEqual) {
   auto query = parseAndConvert("metadata.filter(score >= 0.95).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsDateLessThan) {
   auto query = parseAndConvert("metadata.filter(date_submitted < '2023-01-01'::date).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsDateGreaterEqual) {
   auto query =
      parseAndConvert("metadata.filter(date_submitted >= '2020-06-15'::date).aggregated()");
   ASSERT_NE(query, nullptr);
}

// --- Logical operators ---

TEST(SaneQLAstToQuery, convertsAndExpression) {
   auto query =
      parseAndConvert("metadata.filter(country = 'USA' && region = 'Europe').aggregated()");
   ASSERT_NE(query, nullptr);
   EXPECT_NE(query->filter, nullptr);
}

TEST(SaneQLAstToQuery, convertsOrExpression) {
   auto query =
      parseAndConvert("metadata.filter(country = 'USA' || country = 'Germany').aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsNegation) {
   auto query = parseAndConvert("metadata.filter(!active).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsDoubleNegation) {
   auto query = parseAndConvert("metadata.filter(!!active).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsNestedLogicalExpression) {
   auto query = parseAndConvert(
      "metadata.filter((country = 'USA' || country = 'Germany') && age > 30).aggregated()"
   );
   ASSERT_NE(query, nullptr);
}

// --- Bool literal as filter ---

TEST(SaneQLAstToQuery, convertsTrueLiteral) {
   auto query = parseAndConvert("metadata.filter(true).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsFalseLiteral) {
   auto query = parseAndConvert("metadata.filter(false).aggregated()");
   ASSERT_NE(query, nullptr);
}

// --- Method calls as filters ---

TEST(SaneQLAstToQuery, convertsStringInSet) {
   auto query =
      parseAndConvert("metadata.filter(country.in({'USA', 'Germany', 'France'})).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsIsNull) {
   auto query = parseAndConvert("metadata.filter(host.isNull()).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsIsNotNull) {
   auto query = parseAndConvert("metadata.filter(host.isNotNull()).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsLike) {
   auto query = parseAndConvert("metadata.filter(host.like('Homo.*')).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsIntBetween) {
   auto query = parseAndConvert("metadata.filter(age.between(20, 50)).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsFloatBetween) {
   auto query = parseAndConvert("metadata.filter(score.between(0.5, 1.0)).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsDateBetween) {
   auto query = parseAndConvert(
      "metadata.filter(date_submitted.between('2020-01-01'::date, '2023-12-31'::date)).aggregated()"
   );
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsLineageFilter) {
   auto query = parseAndConvert("metadata.filter(pango_lineage.lineage('B.1.1.7')).aggregated()");
   ASSERT_NE(query, nullptr);
}

// --- Function calls as filters ---

TEST(SaneQLAstToQuery, convertsHasMutationWithNamedArgs) {
   auto query = parseAndConvert(
      "metadata.filter(hasMutation(position:=1000, sequenceName:='main')).aggregated()"
   );
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsHasMutationWithPositionalArg) {
   auto query = parseAndConvert("metadata.filter(hasMutation(1000)).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsHasAAMutation) {
   auto query = parseAndConvert(
      "metadata.filter(hasAAMutation(position:=501, sequenceName:='S')).aggregated()"
   );
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsNOf) {
   auto query = parseAndConvert(
      "metadata.filter(nOf(2, country = 'USA', age > 30, is_complete = true)).aggregated()"
   );
   ASSERT_NE(query, nullptr);
}

// --- Actions ---

TEST(SaneQLAstToQuery, convertsAggregatedWithoutFilter) {
   auto query = parseAndConvert("metadata.aggregated()");
   ASSERT_NE(query, nullptr);
   EXPECT_NE(query->filter, nullptr);
   EXPECT_NE(query->action, nullptr);
}

TEST(SaneQLAstToQuery, convertsAggregatedWithGroupBy) {
   auto query = parseAndConvert("metadata.filter(country = 'USA').aggregated(region)");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsDetailsAction) {
   auto query = parseAndConvert("metadata.filter(country = 'USA').details(limit:=100)");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsDetailsWithOffset) {
   auto query =
      parseAndConvert("metadata.filter(country = 'USA').details(limit:=50, offset:=100)");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsMutationsAction) {
   auto query = parseAndConvert("metadata.filter(country = 'USA').mutations()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsFastaAction) {
   auto query = parseAndConvert("metadata.filter(country = 'USA').fasta()");
   ASSERT_NE(query, nullptr);
}

// --- Full example queries from the plan ---

TEST(SaneQLAstToQuery, convertsFullExampleSimple) {
   auto query = parseAndConvert("metadata.filter(country = 'USA').aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsFullExampleWithMutation) {
   auto query = parseAndConvert(
      "metadata\n"
      "  .filter(hasMutation(position:=1000, sequenceName:='segment1'))\n"
      "  .details(limit:=100)"
   );
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsFullExampleWithDateBetween) {
   auto query = parseAndConvert(
      "metadata\n"
      "  .filter(country = 'USA' && date_submitted.between('2020-01-01'::date, "
      "'2023-12-31'::date))\n"
      "  .aggregated()"
   );
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsFullExampleWithStringInSet) {
   auto query = parseAndConvert(
      "metadata\n"
      "  .filter(country.in({'USA', 'Germany', 'France'}))\n"
      "  .aggregated()"
   );
   ASSERT_NE(query, nullptr);
}

// --- convertToFilter standalone ---

TEST(SaneQLAstToQuery, convertToFilterStringEquals) {
   auto filter = parseFilter("country = 'USA'");
   ASSERT_NE(filter, nullptr);
   EXPECT_EQ(filter->toString(), "country = 'USA'");
}

TEST(SaneQLAstToQuery, convertToFilterIntEquals) {
   auto filter = parseFilter("age = 42");
   ASSERT_NE(filter, nullptr);
}

TEST(SaneQLAstToQuery, convertToFilterAndOr) {
   auto filter = parseFilter("country = 'USA' && (age > 30 || age < 10)");
   ASSERT_NE(filter, nullptr);
}

// --- Error cases ---

TEST(SaneQLAstToQuery, throwsOnMissingAction) {
   Parser parser("metadata.filter(country = 'USA')");
   auto ast = parser.parse();
   EXPECT_THROW(convertToQuery(*ast), ParseException);
}

TEST(SaneQLAstToQuery, throwsOnUnknownAction) {
   Parser parser("metadata.filter(country = 'USA').unknownAction()");
   auto ast = parser.parse();
   EXPECT_THROW(convertToQuery(*ast), ParseException);
}

TEST(SaneQLAstToQuery, throwsOnUnknownFilterMethod) {
   EXPECT_THROW(
      parseAndConvert("metadata.filter(col.unknownMethod()).aggregated()"), ParseException
   );
}

TEST(SaneQLAstToQuery, throwsOnBetweenWrongArgCount) {
   EXPECT_THROW(parseAndConvert("metadata.filter(age.between(1)).aggregated()"), ParseException);
}

TEST(SaneQLAstToQuery, throwsOnInWithNonSet) {
   EXPECT_THROW(parseAndConvert("metadata.filter(col.in('USA')).aggregated()"), ParseException);
}

TEST(SaneQLAstToQuery, throwsOnHasMutationWithoutPosition) {
   EXPECT_THROW(parseAndConvert("metadata.filter(hasMutation()).aggregated()"), ParseException);
}

TEST(SaneQLAstToQuery, throwsOnNOfWithTooFewArgs) {
   EXPECT_THROW(parseAndConvert("metadata.filter(nOf(2)).aggregated()"), ParseException);
}

TEST(SaneQLAstToQuery, throwsOnBareStringLiteral) {
   EXPECT_THROW(parseAndConvert("metadata.filter('USA').aggregated()"), ParseException);
}
