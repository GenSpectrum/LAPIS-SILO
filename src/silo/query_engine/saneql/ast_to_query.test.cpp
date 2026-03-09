#include "silo/query_engine/saneql/ast_to_query.h"

#include <gtest/gtest.h>

#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/query.h"
#include "silo/query_engine/query_tree.h"
#include "silo/query_engine/saneql/parse_exception.h"
#include "silo/query_engine/saneql/parser.h"

using silo::query_engine::QueryNode;
using silo::query_engine::QueryNodePtr;
using silo::query_engine::query_tree::Aggregated;
using silo::query_engine::query_tree::Details;
using silo::query_engine::query_tree::Filter;
using silo::query_engine::query_tree::Limit;
using silo::query_engine::query_tree::OrderBy;
using silo::query_engine::query_tree::TableScan;
using silo::query_engine::saneql::convertToFilter;
using silo::query_engine::saneql::convertToQuery;
using silo::query_engine::saneql::convertToQueryTree;
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

auto parseToTree(const std::string& input) {
   Parser parser(input);
   auto ast = parser.parse();
   return convertToQueryTree(*ast);
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
   auto query =
      parseAndConvert("metadata.filter(date_submitted < '2023-01-01'::date).aggregated()");
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
   auto query = parseAndConvert("metadata.filter(country = 'USA').details(limit:=50, offset:=100)");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsMutationsAction) {
   auto query =
      parseAndConvert("metadata.filter(country = 'USA').mutations(minProportion:='0.05')");
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

// --- New filter functions ---

TEST(SaneQLAstToQuery, convertsNucleotideEquals) {
   auto query = parseAndConvert(
      "metadata.filter(nucleotideEquals(position:=100, symbol:='A', sequenceName:='main'))"
      ".aggregated()"
   );
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsNucleotideEqualsWithDot) {
   auto query =
      parseAndConvert("metadata.filter(nucleotideEquals(position:=100, symbol:='.')).aggregated()");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsAminoAcidEquals) {
   auto query = parseAndConvert(
      "metadata.filter(aminoAcidEquals(position:=501, symbol:='D', sequenceName:='S')).aggregated()"
   );
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsInsertionContains) {
   auto query = parseAndConvert(
      "metadata.filter(insertionContains(position:=12, value:='ACG', sequenceName:='main'))"
      ".aggregated()"
   );
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsAminoAcidInsertionContains) {
   auto query = parseAndConvert(
      "metadata.filter(aminoAcidInsertionContains(position:=5, value:='RN', sequenceName:='S'))"
      ".aggregated()"
   );
   ASSERT_NE(query, nullptr);
}

// --- New action types ---

TEST(SaneQLAstToQuery, convertsFastaAlignedAction) {
   auto query = parseAndConvert("metadata.fastaAligned('main', 'secondary')");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsFastaAlignedWithAdditionalFields) {
   auto query =
      parseAndConvert("metadata.fastaAligned('main', additionalFields:={'country', 'date'})");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsInsertionsAction) {
   auto query = parseAndConvert("metadata.insertions('main')");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsAminoAcidInsertionsAction) {
   auto query = parseAndConvert("metadata.aminoAcidInsertions('S')");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsAminoAcidMutationsAction) {
   auto query = parseAndConvert("metadata.aminoAcidMutations('S', minProportion:='0.05')");
   ASSERT_NE(query, nullptr);
}

// --- Ordering, limit, offset, randomize ---

TEST(SaneQLAstToQuery, convertsOrderBy) {
   auto query = parseAndConvert("metadata.aggregated(country).orderBy('count desc', 'country')");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsDetailsWithLimitOffsetRandomize) {
   auto query = parseAndConvert("metadata.details(limit:=10, offset:=5, randomize:=42)");
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsRandomizeTrue) {
   auto query = parseAndConvert("metadata.details(randomize:=true)");
   ASSERT_NE(query, nullptr);
}

// --- Extended lineage ---

TEST(SaneQLAstToQuery, convertsLineageWithIncludeSublineages) {
   auto query = parseAndConvert(
      "metadata.filter(pango.lineage('B.1.1.7', includeSublineages:=true)).aggregated()"
   );
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsLineageWithNullValue) {
   auto query = parseAndConvert("metadata.filter(pango.lineage(null)).aggregated()");
   ASSERT_NE(query, nullptr);
}

// --- Unknown named argument rejection ---

TEST(SaneQLAstToQuery, throwsOnUnknownNamedArgInAction) {
   EXPECT_THROW(
      parseAndConvert("metadata.aggregated(unknown:=123)"),
      silo::query_engine::IllegalQueryException
   );
}

TEST(SaneQLAstToQuery, throwsOnUnknownNamedArgInFilter) {
   EXPECT_THROW(
      parseAndConvert("metadata.filter(hasMutation(position:=1, unknown:='foo')).aggregated()"),
      silo::query_engine::IllegalQueryException
   );
}

TEST(SaneQLAstToQuery, throwsOnUnknownNamedArgInDetailsAction) {
   EXPECT_THROW(
      parseAndConvert("metadata.details(limt:=10)"), silo::query_engine::IllegalQueryException
   );
}

TEST(SaneQLAstToQuery, throwsOnUnknownNamedArgInLineageFilter) {
   EXPECT_THROW(
      parseAndConvert("metadata.filter(pango.lineage('B.1.1.7', unknown:=true)).aggregated()"),
      silo::query_engine::IllegalQueryException
   );
}

// --- Integration tests via Query::parseQuery ---

using silo::query_engine::IllegalQueryException;
using silo::query_engine::Query;

TEST(SaneQLParseQueryIntegration, parsesValidQuery) {
   auto query = Query::parseQuery("metadata.filter(country = 'USA').aggregated()");
   ASSERT_NE(query, nullptr);
   EXPECT_NE(query->filter, nullptr);
   EXPECT_NE(query->action, nullptr);
}

TEST(SaneQLParseQueryIntegration, wrapsParseErrorAsIllegalQueryException) {
   EXPECT_THROW(Query::parseQuery("not valid saneql ???"), IllegalQueryException);
}

TEST(SaneQLParseQueryIntegration, wrapsSemanticErrorAsIllegalQueryException) {
   EXPECT_THROW(
      Query::parseQuery("metadata.filter(country = 'USA').unknownAction()"), IllegalQueryException
   );
}

// --- Query tree structure tests ---

TEST(SaneQLQueryTree, simpleAggregatedProducesCorrectTree) {
   auto tree = parseToTree("metadata.aggregated()");
   ASSERT_NE(tree, nullptr);
   auto* agg = std::get_if<Aggregated>(&tree->value);
   ASSERT_NE(agg, nullptr);
   EXPECT_TRUE(agg->group_by_fields.empty());
   auto* scan = std::get_if<TableScan>(&agg->source->value);
   ASSERT_NE(scan, nullptr);
}

TEST(SaneQLQueryTree, filterAggregatedProducesFilterNode) {
   auto tree = parseToTree("metadata.filter(country = 'USA').aggregated()");
   ASSERT_NE(tree, nullptr);
   auto* agg = std::get_if<Aggregated>(&tree->value);
   ASSERT_NE(agg, nullptr);
   auto* filter = std::get_if<Filter>(&agg->source->value);
   ASSERT_NE(filter, nullptr);
   EXPECT_NE(filter->predicate, nullptr);
   auto* scan = std::get_if<TableScan>(&filter->source->value);
   ASSERT_NE(scan, nullptr);
}

TEST(SaneQLQueryTree, aggregatedWithGroupByFields) {
   auto tree = parseToTree("metadata.aggregated(country, region)");
   ASSERT_NE(tree, nullptr);
   auto* agg = std::get_if<Aggregated>(&tree->value);
   ASSERT_NE(agg, nullptr);
   ASSERT_EQ(agg->group_by_fields.size(), 2);
   EXPECT_EQ(agg->group_by_fields[0], "country");
   EXPECT_EQ(agg->group_by_fields[1], "region");
}

TEST(SaneQLQueryTree, detailsWithLimitCreatesLimitNode) {
   auto tree = parseToTree("metadata.details(limit:=10)");
   ASSERT_NE(tree, nullptr);
   auto* limit = std::get_if<Limit>(&tree->value);
   ASSERT_NE(limit, nullptr);
   EXPECT_EQ(limit->limit, 10u);
   EXPECT_FALSE(limit->offset.has_value());
   auto* details = std::get_if<Details>(&limit->source->value);
   ASSERT_NE(details, nullptr);
}

TEST(SaneQLQueryTree, detailsWithLimitOffsetRandomize) {
   auto tree = parseToTree("metadata.details(limit:=10, offset:=5, randomize:=42)");
   ASSERT_NE(tree, nullptr);
   auto* limit = std::get_if<Limit>(&tree->value);
   ASSERT_NE(limit, nullptr);
   EXPECT_EQ(limit->limit, 10u);
   EXPECT_EQ(limit->offset, 5u);
   EXPECT_EQ(limit->randomize_seed, 42u);
}

TEST(SaneQLQueryTree, orderByCreatesOrderByNode) {
   auto tree = parseToTree("metadata.aggregated(country).orderBy('count desc', 'country')");
   ASSERT_NE(tree, nullptr);
   auto* order = std::get_if<OrderBy>(&tree->value);
   ASSERT_NE(order, nullptr);
   ASSERT_EQ(order->fields.size(), 2);
   EXPECT_EQ(order->fields[0].name, "count");
   EXPECT_FALSE(order->fields[0].ascending);
   EXPECT_EQ(order->fields[1].name, "country");
   EXPECT_TRUE(order->fields[1].ascending);
   auto* agg = std::get_if<Aggregated>(&order->source->value);
   ASSERT_NE(agg, nullptr);
}

TEST(SaneQLQueryTree, treeRoundTripsToQuery) {
   auto tree = parseToTree("metadata.filter(country = 'USA').aggregated()");
   auto query = silo::query_engine::lowerToQuery(std::move(tree));
   ASSERT_NE(query, nullptr);
   EXPECT_NE(query->filter, nullptr);
   EXPECT_NE(query->action, nullptr);
}
