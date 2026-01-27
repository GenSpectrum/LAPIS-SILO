#include "silo/query_engine/filter/expressions/or.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/filter/expressions/string_equals.h"
#include "silo/query_engine/filter/expressions/string_in_set.h"
#include "silo/query_engine/filter/expressions/symbol_in_set.h"
#include "silo/query_engine/filter/expressions/true.h"
#include "silo/test/query_fixture.test.h"

namespace silo::query_engine::filter::expressions {

namespace {

// Helper to count expressions of a specific type
template <typename T>
size_t countExpressionsOfType(const ExpressionVector& expressions) {
   size_t count = 0;
   for (const auto& expr : expressions) {
      if (dynamic_cast<T*>(expr.get()) != nullptr) {
         count++;
      }
   }
   return count;
}

}  // namespace

// Tests for mergeStringInSetExpressions

TEST(OrMergeStringInSet, shouldMergeTwoStringInSetWithSameColumn) {
   ExpressionVector children;
   children.emplace_back(
      std::make_unique<StringInSet>("country", std::unordered_set<std::string>{"Switzerland"})
   );
   children.emplace_back(
      std::make_unique<StringInSet>("country", std::unordered_set<std::string>{"Germany"})
   );

   auto result = Or::mergeStringInSetExpressions(std::move(children));

   ASSERT_EQ(result.size(), 1);
   auto* merged = dynamic_cast<StringInSet*>(result[0].get());
   ASSERT_NE(merged, nullptr);
   const std::string str = merged->toString();
   // Format is "column IN [value1,value2,...]"
   EXPECT_NE(str.find("country IN ["), std::string::npos);
   EXPECT_NE(str.find("Switzerland"), std::string::npos);
   EXPECT_NE(str.find("Germany"), std::string::npos);
}

TEST(OrMergeStringInSet, shouldKeepSeparateStringInSetWithDifferentColumns) {
   ExpressionVector children;
   children.emplace_back(
      std::make_unique<StringInSet>("country", std::unordered_set<std::string>{"Switzerland"})
   );
   children.emplace_back(
      std::make_unique<StringInSet>("region", std::unordered_set<std::string>{"Europe"})
   );

   auto result = Or::mergeStringInSetExpressions(std::move(children));

   ASSERT_EQ(result.size(), 2);
   EXPECT_EQ(countExpressionsOfType<StringInSet>(result), 2);
}

TEST(OrMergeStringInSet, shouldPassThroughOtherExpressions) {
   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(
      std::make_unique<StringInSet>("country", std::unordered_set<std::string>{"Switzerland"})
   );
   children.emplace_back(std::make_unique<True>());

   auto result = Or::mergeStringInSetExpressions(std::move(children));

   ASSERT_EQ(result.size(), 3);
   EXPECT_EQ(countExpressionsOfType<True>(result), 2);
   EXPECT_EQ(countExpressionsOfType<StringInSet>(result), 1);
}

TEST(OrMergeStringInSet, shouldHandleEmptyInput) {
   ExpressionVector children;

   auto result = Or::mergeStringInSetExpressions(std::move(children));

   EXPECT_TRUE(result.empty());
}

TEST(OrMergeStringInSet, shouldHandleSingleStringInSet) {
   ExpressionVector children;
   children.emplace_back(
      std::make_unique<StringInSet>("country", std::unordered_set<std::string>{"Switzerland"})
   );

   auto result = Or::mergeStringInSetExpressions(std::move(children));

   ASSERT_EQ(result.size(), 1);
   EXPECT_NE(dynamic_cast<StringInSet*>(result[0].get()), nullptr);
}

TEST(OrMergeStringInSet, shouldMergeMultipleValuesFromMultipleExpressions) {
   ExpressionVector children;
   children.emplace_back(std::make_unique<StringInSet>(
      "country", std::unordered_set<std::string>{"Switzerland", "Austria"}
   ));
   children.emplace_back(
      std::make_unique<StringInSet>("country", std::unordered_set<std::string>{"Germany", "France"})
   );
   children.emplace_back(
      std::make_unique<StringInSet>("country", std::unordered_set<std::string>{"Italy"})
   );

   auto result = Or::mergeStringInSetExpressions(std::move(children));

   ASSERT_EQ(result.size(), 1);
   auto* merged = dynamic_cast<StringInSet*>(result[0].get());
   ASSERT_NE(merged, nullptr);
   const std::string str = merged->toString();
   EXPECT_NE(str.find("Switzerland"), std::string::npos);
   EXPECT_NE(str.find("Germany"), std::string::npos);
   EXPECT_NE(str.find("France"), std::string::npos);
   EXPECT_NE(str.find("Austria"), std::string::npos);
   EXPECT_NE(str.find("Italy"), std::string::npos);
}

TEST(OrMergeStringInSet, shouldHandleDuplicateValues) {
   ExpressionVector children;
   children.emplace_back(
      std::make_unique<StringInSet>("country", std::unordered_set<std::string>{"Switzerland"})
   );
   children.emplace_back(std::make_unique<StringInSet>(
      "country", std::unordered_set<std::string>{"Switzerland", "Germany"}
   ));

   auto result = Or::mergeStringInSetExpressions(std::move(children));

   ASSERT_EQ(result.size(), 1);
   auto* merged = dynamic_cast<StringInSet*>(result[0].get());
   ASSERT_NE(merged, nullptr);
   const std::string str = merged->toString();
   EXPECT_NE(str.find("Switzerland"), std::string::npos);
   EXPECT_NE(str.find("Germany"), std::string::npos);
}

// Tests for rewriteSymbolInSetExpressions

TEST(OrRewriteSymbolInSet, shouldMergeTwoSymbolInSetWithSamePosition) {
   ExpressionVector children;
   children.emplace_back(std::make_unique<SymbolInSet<Nucleotide>>(
      std::nullopt, 100, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::A}
   ));
   children.emplace_back(std::make_unique<SymbolInSet<Nucleotide>>(
      std::nullopt, 100, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::G}
   ));

   auto result = Or::rewriteSymbolInSetExpressions<Nucleotide>(std::move(children));

   ASSERT_EQ(result.size(), 1);
   auto* merged = dynamic_cast<SymbolInSet<Nucleotide>*>(result[0].get());
   ASSERT_NE(merged, nullptr);
   // Format is "(sequence_name:symbol at position X in {A, G, ...})"
   const std::string str = merged->toString();
   EXPECT_NE(str.find("position 101"), std::string::npos);
   EXPECT_NE(str.find("A"), std::string::npos);
   EXPECT_NE(str.find("G"), std::string::npos);
}

TEST(OrRewriteSymbolInSet, shouldKeepSeparateSymbolInSetWithDifferentPositions) {
   ExpressionVector children;
   children.emplace_back(std::make_unique<SymbolInSet<Nucleotide>>(
      std::nullopt, 100, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::A}
   ));
   children.emplace_back(std::make_unique<SymbolInSet<Nucleotide>>(
      std::nullopt, 200, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::G}
   ));

   auto result = Or::rewriteSymbolInSetExpressions<Nucleotide>(std::move(children));

   ASSERT_EQ(result.size(), 2);
   EXPECT_EQ(countExpressionsOfType<SymbolInSet<Nucleotide>>(result), 2);
}

TEST(OrRewriteSymbolInSet, shouldKeepSeparateSymbolInSetWithDifferentSequenceNames) {
   ExpressionVector children;
   children.emplace_back(std::make_unique<SymbolInSet<Nucleotide>>(
      "segment1", 100, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::A}
   ));
   children.emplace_back(std::make_unique<SymbolInSet<Nucleotide>>(
      "segment2", 100, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::G}
   ));

   auto result = Or::rewriteSymbolInSetExpressions<Nucleotide>(std::move(children));

   ASSERT_EQ(result.size(), 2);
   EXPECT_EQ(countExpressionsOfType<SymbolInSet<Nucleotide>>(result), 2);
}

TEST(OrRewriteSymbolInSet, shouldPassThroughOtherExpressions) {
   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<SymbolInSet<Nucleotide>>(
      std::nullopt, 100, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::A}
   ));
   children.emplace_back(std::make_unique<True>());

   auto result = Or::rewriteSymbolInSetExpressions<Nucleotide>(std::move(children));

   ASSERT_EQ(result.size(), 3);
   EXPECT_EQ(countExpressionsOfType<True>(result), 2);
   EXPECT_EQ(countExpressionsOfType<SymbolInSet<Nucleotide>>(result), 1);
}

TEST(OrRewriteSymbolInSet, shouldHandleEmptyInput) {
   ExpressionVector children;

   auto result = Or::rewriteSymbolInSetExpressions<Nucleotide>(std::move(children));

   EXPECT_TRUE(result.empty());
}

TEST(OrRewriteSymbolInSet, shouldMergeMultipleSymbolsFromMultipleExpressions) {
   ExpressionVector children;
   children.emplace_back(std::make_unique<SymbolInSet<Nucleotide>>(
      std::nullopt, 100, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::A}
   ));
   children.emplace_back(std::make_unique<SymbolInSet<Nucleotide>>(
      std::nullopt,
      100,
      std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::G, Nucleotide::Symbol::C}
   ));
   children.emplace_back(std::make_unique<SymbolInSet<Nucleotide>>(
      std::nullopt, 100, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::T}
   ));

   auto result = Or::rewriteSymbolInSetExpressions<Nucleotide>(std::move(children));

   ASSERT_EQ(result.size(), 1);
   auto* merged = dynamic_cast<SymbolInSet<Nucleotide>*>(result[0].get());
   ASSERT_NE(merged, nullptr);
   const std::string str = merged->toString();
   EXPECT_NE(str.find("A"), std::string::npos);
   EXPECT_NE(str.find("G"), std::string::npos);
   EXPECT_NE(str.find("C"), std::string::npos);
   EXPECT_NE(str.find("T"), std::string::npos);
}

TEST(OrRewriteSymbolInSet, shouldMergeOnlyMatchingPositionsAndSequences) {
   ExpressionVector children;
   // These two should merge (same position, nullopt sequence)
   children.emplace_back(std::make_unique<SymbolInSet<Nucleotide>>(
      std::nullopt, 100, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::A}
   ));
   children.emplace_back(std::make_unique<SymbolInSet<Nucleotide>>(
      std::nullopt, 100, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::G}
   ));
   // This one stays separate (different position)
   children.emplace_back(std::make_unique<SymbolInSet<Nucleotide>>(
      std::nullopt, 200, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::C}
   ));
   // These two should merge (same position and sequence name)
   children.emplace_back(std::make_unique<SymbolInSet<Nucleotide>>(
      "segment1", 50, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::A}
   ));
   children.emplace_back(std::make_unique<SymbolInSet<Nucleotide>>(
      "segment1", 50, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::T}
   ));

   auto result = Or::rewriteSymbolInSetExpressions<Nucleotide>(std::move(children));

   // Should have 3 SymbolInSet expressions:
   // 1. Merged A+G at position 100 (nullopt)
   // 2. C at position 200 (nullopt)
   // 3. Merged A+T at position 50 (segment1)
   ASSERT_EQ(result.size(), 3);
   EXPECT_EQ(countExpressionsOfType<SymbolInSet<Nucleotide>>(result), 3);
}

// Tests for Or::toString

TEST(OrToString, shouldFormatChildrenCorrectly) {
   ExpressionVector children;
   children.emplace_back(std::make_unique<True>());
   children.emplace_back(std::make_unique<True>());

   Or or_expression(std::move(children));

   EXPECT_EQ(or_expression.toString(), "Or(True | True)");
}

using silo::query_engine::filter::expressions::StringEquals;
using silo::schema::ColumnIdentifier;
using silo::schema::ColumnType;
using silo::storage::column::ColumnMetadata;
using silo::storage::column::StringColumnMetadata;

TEST(OrToString, shouldHandleNestedStringEquals) {
   ColumnIdentifier primary_key{.name = "key", .type = ColumnType::STRING};
   std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> column_metadata{
      {primary_key, std::make_shared<StringColumnMetadata>(primary_key.name)}
   };
   silo::storage::Table table(schema::TableSchema{column_metadata, primary_key});
   auto table_partition = table.addPartition();

   ExpressionVector inner_children;
   inner_children.emplace_back(std::make_unique<StringEquals>("key", "value_1"));
   inner_children.emplace_back(std::make_unique<StringEquals>("key", "value_2"));

   ExpressionVector outer_children;
   outer_children.emplace_back(std::make_unique<Or>(std::move(inner_children)));
   outer_children.emplace_back(std::make_unique<StringEquals>("key", "value_3"));

   Or outer_or(std::move(outer_children));

   EXPECT_EQ(outer_or.toString(), "Or(Or(key = 'value_1' | key = 'value_2') | key = 'value_3')");

   auto rewritten_or = outer_or.rewrite(table, *table_partition, Or::AmbiguityMode::NONE);

   EXPECT_EQ(rewritten_or->toString(), "key IN [value_1,value_2,value_3]");
}

}  // namespace silo::query_engine::filter::expressions

// Query tests for nested Or expressions and compiled trees
namespace {

using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

nlohmann::json createData(const std::string& primary_key, const std::string& country) {
   return nlohmann::json::parse(fmt::format(
      R"(
{{
   "primaryKey": "{}",
   "country": "{}",
   "region": "{}",
   "segment1": {{
      "sequence": "ACGT",
      "insertions": []
   }}
}}
)",
      primary_key,
      country,
      country == "Switzerland" ? "Europe" : (country == "USA" ? "Americas" : "Europe")
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
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "ATGCN"}},
   {},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {createData("id_0", "Switzerland"),
       createData("id_1", "Germany"),
       createData("id_2", "USA"),
       createData("id_3", "Switzerland"),
       createData("id_4", "France"),
       createData("id_5", "Germany")},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .without_unaligned_sequences = true
};

// Test nested Or expressions - inner Or should be flattened during compilation
const QueryTestScenario NESTED_OR_SAME_COLUMN = {
   .name = "NESTED_OR_SAME_COLUMN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey", "country"]
  },
  "filterExpression": {
    "type": "Or",
    "children": [
      {
        "type": "Or",
        "children": [
          {"type": "StringEquals", "column": "country", "value": "Switzerland"},
          {"type": "StringEquals", "column": "country", "value": "Germany"}
        ]
      },
      {"type": "StringEquals", "column": "country", "value": "France"}
    ]
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"Switzerland","primaryKey":"id_0"},
{"country":"Germany","primaryKey":"id_1"},
{"country":"Switzerland","primaryKey":"id_3"},
{"country":"France","primaryKey":"id_4"},
{"country":"Germany","primaryKey":"id_5"
}])"
   )
};

// Test deeply nested Or expressions
const QueryTestScenario DEEPLY_NESTED_OR = {
   .name = "DEEPLY_NESTED_OR",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey", "country"]
  },
  "filterExpression": {
    "type": "Or",
    "children": [
      {
        "type": "Or",
        "children": [
          {
            "type": "Or",
            "children": [
              {"type": "StringEquals", "column": "country", "value": "Switzerland"}
            ]
          },
          {"type": "StringEquals", "column": "country", "value": "Germany"}
        ]
      }
    ]
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"Switzerland","primaryKey":"id_0"},
{"country":"Germany","primaryKey":"id_1"},
{"country":"Switzerland","primaryKey":"id_3"},
{"country":"Germany","primaryKey":"id_5"}
])"
   )
};

// Test Or with single child gets unwrapped during rewrite
const QueryTestScenario OR_SINGLE_CHILD_UNWRAPPED = {
   .name = "OR_SINGLE_CHILD_UNWRAPPED",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey", "country"]
  },
  "filterExpression": {
    "type": "Or",
    "children": [
      {"type": "StringEquals", "column": "country", "value": "Switzerland"}
    ]
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([{"country":"Switzerland","primaryKey":"id_0"},{"country":"Switzerland","primaryKey":"id_3"}])"
   )
};

// Test Or with multiple StringEquals on same indexed column get merged
const QueryTestScenario OR_STRING_EQUALS_MERGED = {
   .name = "OR_STRING_EQUALS_MERGED",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey", "country"]
  },
  "filterExpression": {
    "type": "Or",
    "children": [
      {"type": "StringEquals", "column": "country", "value": "Switzerland"},
      {"type": "StringEquals", "column": "country", "value": "Germany"},
      {"type": "StringEquals", "column": "country", "value": "France"}
    ]
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"Switzerland","primaryKey":"id_0"},
{"country":"Germany","primaryKey":"id_1"},
{"country":"Switzerland","primaryKey":"id_3"},
{"country":"France","primaryKey":"id_4"},
{"country":"Germany","primaryKey":"id_5"}
])"
   )
};

// Test Or with mixed columns - should not merge different columns
const QueryTestScenario OR_MIXED_COLUMNS = {
   .name = "OR_MIXED_COLUMNS",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey", "country", "region"]
  },
  "filterExpression": {
    "type": "Or",
    "children": [
      {"type": "StringEquals", "column": "country", "value": "USA"},
      {"type": "StringEquals", "column": "region", "value": "Europe"}
    ]
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"Switzerland","primaryKey":"id_0","region":"Europe"},
{"country":"Germany","primaryKey":"id_1","region":"Europe"},
{"country":"USA","primaryKey":"id_2","region":"Americas"},
{"country":"Switzerland","primaryKey":"id_3","region":"Europe"},
{"country":"France","primaryKey":"id_4","region":"Europe"},
{"country":"Germany","primaryKey":"id_5","region":"Europe"}
])"
   )
};

// Test nested Or with And - should not flatten Or across And
const QueryTestScenario OR_WITH_AND = {
   .name = "OR_WITH_AND",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey", "country", "region"]
  },
  "filterExpression": {
    "type": "Or",
    "children": [
      {
        "type": "And",
        "children": [
          {"type": "StringEquals", "column": "country", "value": "Switzerland"},
          {"type": "StringEquals", "column": "region", "value": "Europe"}
        ]
      },
      {"type": "StringEquals", "column": "country", "value": "USA"}
    ]
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([
{"country":"Switzerland","primaryKey":"id_0","region":"Europe"},
{"country":"USA","primaryKey":"id_2","region":"Americas"},
{"country":"Switzerland","primaryKey":"id_3","region":"Europe"}
])"
   )
};

// Test empty Or returns empty result
const QueryTestScenario OR_EMPTY_CHILDREN = {
   .name = "OR_EMPTY_CHILDREN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey"]
  },
  "filterExpression": {
    "type": "Or",
    "children": []
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(R"([])")
};

}  // namespace

QUERY_TEST(
   OrRewrite,
   TEST_DATA,
   ::testing::Values(
      NESTED_OR_SAME_COLUMN,
      DEEPLY_NESTED_OR,
      OR_SINGLE_CHILD_UNWRAPPED,
      OR_STRING_EQUALS_MERGED,
      OR_MIXED_COLUMNS,
      OR_WITH_AND,
      OR_EMPTY_CHILDREN
   )
);
