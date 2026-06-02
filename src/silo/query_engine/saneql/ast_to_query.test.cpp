#include "silo/query_engine/saneql/ast_to_query.h"

#include <map>
#include <memory>
#include <string_view>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/saneql/ast.h"
#include "silo/query_engine/saneql/parser.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/table.h"

using silo::query_engine::IllegalQueryException;
using silo::query_engine::saneql::convertToFilter;
using silo::query_engine::saneql::parseAndConvertToQueryTree;
using silo::query_engine::saneql::Parser;
namespace ast = silo::query_engine::saneql::ast;

namespace {

auto parseFilter(std::string_view query) {
   return convertToFilter(*Parser(query).parse());
}

using Tables = std::map<silo::schema::TableName, std::shared_ptr<silo::storage::Table>>;

Tables makeTablesWithDefault() {
   using silo::schema::ColumnIdentifier;
   using silo::schema::ColumnType;
   using silo::storage::column::ColumnMetadata;
   using silo::storage::column::StringColumnMetadata;

   ColumnIdentifier primary_key{.name = "id", .type = ColumnType::STRING};
   std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> col_meta{
      {primary_key, std::make_shared<StringColumnMetadata>(primary_key.name)}
   };
   auto schema = std::make_shared<silo::schema::TableSchema>(std::move(col_meta), primary_key);
   Tables tables;
   const silo::schema::TableName table_name("default");
   tables[table_name] = std::make_shared<silo::storage::Table>(table_name, schema);
   return tables;
}

// --- between ---

TEST(AstToQuery, nullFromAndTo) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&tables]() {
         (void)parseAndConvertToQueryTree("default.filter(id.between(null, null))", tables);
      },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("Could not infer type of between expression. From-value or to-value "
                              "needs to be a typed non-null value, got from: 'null' to: 'null'")
      )
   );
}

// --- nucleotideEquals ---

TEST(AstToQueryNucleotideEquals, multiCharSymbolThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("nucleotideEquals(position:=1, symbol:='ZZ')"); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("nucleotideEquals() symbol must be a single character")
      )
   );
}

TEST(AstToQueryNucleotideEquals, invalidSymbolThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("nucleotideEquals(position:=1, symbol:='Q')"); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("nucleotideEquals() invalid symbol 'Q'")
      )
   );
}

// --- lineage ---

TEST(AstToQueryLineage, invalidRecombinantFollowingModeThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("lineage(clade, 'XBB', recombinantFollowingMode:='badMode')"); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("invalid recombinantFollowingMode: 'badMode'")
      )
   );
}

// --- nucleotideMutationProfile ---

TEST(AstToQueryMutationProfile, mutationsNotSetLiteralThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("nucleotideMutationProfile(distance:=1, mutations:='ACGT')"); },
      ThrowsMessage<IllegalQueryException>(::testing::HasSubstr(
         "The 'mutations' argument of a Nucleotide MutationProfile expression must be a set "
         "literal"
      ))
   );
}

TEST(AstToQueryMutationProfile, mutationRecordPositionZeroThrows) {
   EXPECT_THAT(
      []() {
         (void)parseFilter(
            "nucleotideMutationProfile(distance:=1, mutations:={{position:=0, symbol:='A'}})"
         );
      },
      ThrowsMessage<IllegalQueryException>(::testing::HasSubstr("value 0 is not allowed"))
   );
}

TEST(AstToQueryMutationProfile, mutationRecordMissingPositionThrows) {
   EXPECT_THAT(
      []() {
         (void)parseFilter("nucleotideMutationProfile(distance:=1, mutations:={{symbol:='A'}})");
      },
      ThrowsMessage<IllegalQueryException>(::testing::HasSubstr("must have a 'position' field"))
   );
}

TEST(AstToQueryMutationProfile, mutationRecordMissingSymbolThrows) {
   EXPECT_THAT(
      []() {
         (void)parseFilter("nucleotideMutationProfile(distance:=1, mutations:={{position:=1}})");
      },
      ThrowsMessage<IllegalQueryException>(::testing::HasSubstr("must have a 'symbol' field"))
   );
}

TEST(AstToQueryMutationProfile, mutationRecordMultiCharSymbolThrows) {
   EXPECT_THAT(
      []() {
         (void)parseFilter(
            "nucleotideMutationProfile(distance:=1, mutations:={{position:=1, symbol:='AB'}})"
         );
      },
      ThrowsMessage<IllegalQueryException>(::testing::HasSubstr("must be a single character"))
   );
}

TEST(AstToQueryMutationProfile, mutationRecordInvalidSymbolThrows) {
   EXPECT_THAT(
      []() {
         (void)parseFilter(
            "nucleotideMutationProfile(distance:=1, mutations:={{position:=1, symbol:='Q'}})"
         );
      },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("Invalid Nucleotide symbol 'Q' in MutationProfile")
      )
   );
}

TEST(AstToQueryMutationProfile, mutationListElementNotRecordThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("nucleotideMutationProfile(distance:=1, mutations:={'A123T'})"); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("must be a record literal with 'position' and 'symbol' fields")
      )
   );
}

// --- convertEqualsToFilter ---

TEST(AstToQueryConvertEqualsToFilter, unsupportedValueTypeThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("a = {1, 2}"); },
      ThrowsMessage<IllegalQueryException>(::testing::HasSubstr("unsupported value type in equality"
      ))
   );
}

// --- convertToFilter ---

TEST(AstToQueryConvertToFilter, unknownScalarFunctionThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("unknownFunc(1)"); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("unknown scalar function 'unknownFunc'")
      )
   );
}

TEST(AstToQueryConvertToFilter, unsupportedExpressionTypeThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("42"); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("unsupported expression type in filter context")
      )
   );
}

// --- integer comparisons ---

TEST(AstToQueryIntComparison, lessThanThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("age < 5"); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("less than is not implemented for integer expressions")
      )
   );
}

TEST(AstToQueryIntComparison, greaterThanThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("age > 5"); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("greater than is not implemented for integer expressions")
      )
   );
}

// --- float comparisons ---

TEST(AstToQueryFloatComparison, lessEqualThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("age <= 5.0"); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("less equal is not implemented for float expressions")
      )
   );
}

TEST(AstToQueryFloatComparison, greaterThanThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("age > 5.0"); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("greater than is not implemented for float expressions")
      )
   );
}

// --- date comparisons ---

TEST(AstToQueryDateComparison, lessThanThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("date < '2020-01-01'::date"); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("less than is not implemented for date expressions")
      )
   );
}

TEST(AstToQueryDateComparison, greaterThanThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("date > '2020-01-01'::date"); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("greater than is not implemented for date expressions")
      )
   );
}

// --- convertBinaryExprToFilter ---

TEST(AstToQueryBinaryExpr, equalsNoIdentifierThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("'a' = 'b'"); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("equality comparison requires an identifier on one side")
      )
   );
}

TEST(AstToQueryBinaryExpr, notEqualsNoIdentifierThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("'a' <> 'b'"); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("not-equals comparison requires an identifier on one side")
      )
   );
}

TEST(AstToQueryBinaryExpr, comparisonNoIdentifierLeftThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("1 < age"); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("comparison requires an identifier on the left side")
      )
   );
}

TEST(AstToQueryBinaryExpr, unhandledBinaryOpThrows) {
   // All 8 BinaryOp enum values are handled in convertBinaryExprToFilter; the throw
   // after the switch is a defensive guard reachable only via an out-of-range op value.
   auto expr = ast::makeExpr(
      ast::BinaryExpr{
         .op = static_cast<ast::BinaryOp>(99),
         .left = ast::makeExpr(ast::Identifier{"a"}, {}),
         .right = ast::makeExpr(ast::IntLiteral{1}, {}),
      },
      {}
   );
   EXPECT_THAT(
      [&]() { (void)convertToFilter(*expr); },
      ThrowsMessage<IllegalQueryException>(::testing::HasSubstr("unhandled binary operator"))
   );
}

// --- groupBy ---

TEST(AstToQueryGroupBy, aggregatesNotRecordLiteralThrows) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&]() { (void)parseAndConvertToQueryTree("default.groupBy('not_a_record')", tables); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("groupBy aggregates must be a record literal")
      )
   );
}

TEST(AstToQueryGroupBy, aggregateDefNotFunctionCallThrows) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&]() { (void)parseAndConvertToQueryTree("default.groupBy({n:=42})", tables); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("aggregate definition 'n' must be a function call")
      )
   );
}

TEST(AstToQueryGroupBy, unknownAggregateFunctionThrows) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&]() { (void)parseAndConvertToQueryTree("default.groupBy({n:=sum()})", tables); },
      ThrowsMessage<IllegalQueryException>(::testing::HasSubstr("unknown aggregate function 'sum'"))
   );
}

TEST(AstToQueryGroupBy, fieldNotInSchemaThrows) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&tables]() {
         (void)parseAndConvertToQueryTree("default.groupBy({n:=count()}, {nonexistent})", tables);
      },
      ThrowsMessage<IllegalQueryException>(::testing::HasSubstr(
         "groupBy field 'nonexistent' is not present in the input's output schema"
      ))
   );
}

// --- project ---

TEST(AstToQueryProject, fieldNotInSchemaThrows) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&tables]() { (void)parseAndConvertToQueryTree("default.project(nonexistent)", tables); },
      ThrowsMessage<IllegalQueryException>(::testing::HasSubstr(
         "project field 'nonexistent' is not present in the input's output schema"
      ))
   );
}

// --- map ---

TEST(AstToQueryMap, expressionsNotRecordLiteralThrows) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&tables]() { (void)parseAndConvertToQueryTree("default.map(id)", tables); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("map() expects a record of assignments like {x := 3, y := age}")
      )
   );
}

TEST(AstToQueryMap, emptyBracesAreNotARecordLiteral) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&tables]() { (void)parseAndConvertToQueryTree("default.map({})", tables); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("map() expects a record of assignments like {x := 3, y := age}")
      )
   );
}

TEST(AstToQueryMap, fieldReferenceRejectedForNow) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&tables]() { (void)parseAndConvertToQueryTree("default.map({x := id})", tables); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("map() field 'x' must be assigned a literal value")
      )
   );
}

TEST(AstToQueryMap, unsupportedValueThrows) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&tables]() { (void)parseAndConvertToQueryTree("default.map({x := count()})", tables); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("map() field 'x' must be assigned a literal value")
      )
   );
}

// --- orderBy ---

TEST(AstToQueryOrderBy, fieldUnsupportedTypeThrows) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&tables]() { (void)parseAndConvertToQueryTree("default.orderBy({'value'})", tables); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("orderBy field must be an identifier or asc()/desc() call")
      )
   );
}

TEST(AstToQueryOrderBy, unsupportedFunctionNameThrows) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&tables]() { (void)parseAndConvertToQueryTree("default.orderBy({foo(bar)})", tables); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("orderBy field must be an identifier or asc()/desc() call, got 'foo'")
      )
   );
}

TEST(AstToQueryOrderBy, ascWrongArgCountThrows) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&]() { (void)parseAndConvertToQueryTree("default.orderBy({asc()})", tables); },
      ThrowsMessage<IllegalQueryException>(::testing::HasSubstr("asc() expects exactly one argument"
      ))
   );
}

TEST(AstToQueryOrderBy, unknownFieldThrows) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&tables]() { (void)parseAndConvertToQueryTree("default.orderBy({nonexistent})", tables); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("OrderByField nonexistent is not contained in the result")
      )
   );
}

TEST(AstToQueryOrderBy, unknownFieldInAscThrows) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&tables]() {
         (void)parseAndConvertToQueryTree("default.orderBy({asc(nonexistent)})", tables);
      },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("OrderByField nonexistent is not contained in the result")
      )
   );
}

// --- limit ---

TEST(AstToQueryLimit, zeroLimitThrows) {
   auto tables = makeTablesWithDefault();
   EXPECT_THAT(
      [&tables]() { (void)parseAndConvertToQueryTree("default.limit(0)", tables); },
      ThrowsMessage<IllegalQueryException>(::testing::HasSubstr("limit must be a positive number"))
   );
}

// --- buildScanNode ---

TEST(AstToQueryBuildScanNode, tableNotFoundThrows) {
   EXPECT_THAT(
      []() { (void)parseAndConvertToQueryTree("unknown", {}); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("table 'unknown' not found in database")
      )
   );
}

// --- nOf ---

TEST(AstToQueryNOf, countNotIntegerThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("nOf('two', {country = 'Switzerland'})"); },
      ThrowsMessage<IllegalQueryException>(::testing::HasSubstr("expected integer literal"))
   );
}

TEST(AstToQueryNOf, childrenNotSetLiteralThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("nOf(1, country = 'Switzerland')"); },
      ThrowsMessage<IllegalQueryException>(::testing::HasSubstr("expected set literal"))
   );
}

TEST(AstToQueryNOf, matchExactlyNotBoolThrows) {
   EXPECT_THAT(
      []() { (void)parseFilter("nOf(1, {country = 'Switzerland'}, matchExactly:='yes')"); },
      ThrowsMessage<IllegalQueryException>(::testing::HasSubstr("expected boolean literal"))
   );
}

// --- convertExpression ---

TEST(AstToQueryConvertExpression, nonIdentifierNonFunctionCallThrows) {
   EXPECT_THAT(
      []() { (void)parseAndConvertToQueryTree("42", {}); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("expected table reference or function call")
      )
   );
}

}  // namespace
