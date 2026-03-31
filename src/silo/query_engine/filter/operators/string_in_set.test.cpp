#include "silo/query_engine/filter/operators/string_in_set.h"

#include <unordered_set>

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

#include "silo/query_engine/filter/operators/selection.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/string_column.h"

using silo::query_engine::filter::operators::Selection;
using silo::query_engine::filter::operators::StringInSet;
using silo::storage::column::IndexedStringColumn;
using silo::storage::column::IndexedStringColumnMetadata;
using silo::storage::column::StringColumn;
using silo::storage::column::StringColumnMetadata;

namespace {

std::pair<std::shared_ptr<StringColumnMetadata>, StringColumn> makeTestStringColumn(
   const std::vector<std::string>& values
) {
   auto metadata = std::make_shared<StringColumnMetadata>("test");
   StringColumn test_column{metadata.get()};
   for (const auto& value : values) {
      SILO_ASSERT(test_column.insert(value).has_value());
   }
   return {metadata, std::move(test_column)};
}

std::pair<std::shared_ptr<IndexedStringColumnMetadata>, IndexedStringColumn>
makeTestIndexedStringColumn(const std::vector<std::string>& values) {
   auto metadata = std::make_shared<IndexedStringColumnMetadata>("test_indexed");
   IndexedStringColumn test_column{metadata.get()};
   for (const auto& value : values) {
      SILO_ASSERT(test_column.insert(value).has_value());
   }
   return {metadata, std::move(test_column)};
}

}  // namespace

TEST(OperatorStringInSet, matchReturnsCorrectValuesForStringColumn) {
   const std::vector<std::string> values{
      {"Switzerland", "Germany", "USA", "Switzerland", "France", "Germany"}
   };
   auto [metadata, test_column] = makeTestStringColumn(values);
   const uint32_t row_count = values.size();

   auto under_test = std::make_unique<Selection>(
      std::make_unique<StringInSet<StringColumn>>(
         &test_column,
         StringInSet<StringColumn>::Comparator::IN,
         std::unordered_set<std::string>{"Switzerland", "Germany"}
      ),
      row_count
   );

   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({0, 1, 3, 5}));
}

TEST(OperatorStringInSet, matchReturnsCorrectValuesForIndexedStringColumn) {
   const std::vector<std::string> values{
      {"Switzerland", "Germany", "USA", "Switzerland", "France", "Germany"}
   };
   auto [metadata, test_column] = makeTestIndexedStringColumn(values);
   const uint32_t row_count = values.size();

   auto under_test = std::make_unique<Selection>(
      std::make_unique<StringInSet<IndexedStringColumn>>(
         &test_column,
         StringInSet<IndexedStringColumn>::Comparator::IN,
         std::unordered_set<std::string>{"Switzerland", "Germany"}
      ),
      row_count
   );

   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({0, 1, 3, 5}));
}

TEST(OperatorStringInSet, matchReturnsEmptyForNoMatches) {
   const std::vector<std::string> values{
      {"Switzerland", "Germany", "USA", "Switzerland", "France", "Germany"}
   };
   auto [metadata, test_column] = makeTestStringColumn(values);
   const uint32_t row_count = values.size();

   auto under_test = std::make_unique<Selection>(
      std::make_unique<StringInSet<StringColumn>>(
         &test_column,
         StringInSet<StringColumn>::Comparator::IN,
         std::unordered_set<std::string>{"Japan", "China"}
      ),
      row_count
   );

   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring());
}

TEST(OperatorStringInSet, matchReturnsEmptyForEmptySet) {
   const std::vector<std::string> values{
      {"Switzerland", "Germany", "USA", "Switzerland", "France", "Germany"}
   };
   auto [metadata, test_column] = makeTestStringColumn(values);
   const uint32_t row_count = values.size();

   auto under_test = std::make_unique<Selection>(
      std::make_unique<StringInSet<StringColumn>>(
         &test_column, StringInSet<StringColumn>::Comparator::IN, std::unordered_set<std::string>{}
      ),
      row_count
   );

   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring());
}

TEST(OperatorStringInSet, negationWorksCorrectly) {
   const std::vector<std::string> values{
      {"Switzerland", "Germany", "USA", "Switzerland", "France", "Germany"}
   };
   auto [metadata, test_column] = makeTestStringColumn(values);
   const uint32_t row_count = values.size();

   auto under_test = std::make_unique<Selection>(
      std::make_unique<StringInSet<StringColumn>>(
         &test_column,
         StringInSet<StringColumn>::Comparator::IN,
         std::unordered_set<std::string>{"Switzerland", "Germany"}
      ),
      row_count
   );

   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({0, 1, 3, 5}));

   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(negated->evaluate().getConstReference(), roaring::Roaring({2, 4}));
}

TEST(OperatorStringInSet, notInComparatorWorksCorrectly) {
   const std::vector<std::string> values{
      {"Switzerland", "Germany", "USA", "Switzerland", "France", "Germany"}
   };
   auto [metadata, test_column] = makeTestStringColumn(values);
   const uint32_t row_count = values.size();

   auto under_test = std::make_unique<Selection>(
      std::make_unique<StringInSet<StringColumn>>(
         &test_column,
         StringInSet<StringColumn>::Comparator::NOT_IN,
         std::unordered_set<std::string>{"Switzerland", "Germany"}
      ),
      row_count
   );

   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({2, 4}));
}

TEST(OperatorStringInSet, toStringReturnsCorrectFormat) {
   const std::vector<std::string> values{"Switzerland", "Germany"};
   auto [metadata, test_column] = makeTestStringColumn(values);

   const StringInSet<StringColumn> in_predicate(
      &test_column,
      StringInSet<StringColumn>::Comparator::IN,
      std::unordered_set<std::string>{"Value"}
   );

   ASSERT_EQ(in_predicate.toString(), "test IN [Value]");

   const StringInSet<StringColumn> not_in_predicate(
      &test_column,
      StringInSet<StringColumn>::Comparator::NOT_IN,
      std::unordered_set<std::string>{"Value"}
   );

   ASSERT_EQ(not_in_predicate.toString(), "test NOT IN [Value]");
}

TEST(OperatorStringInSet, copyCreatesIndependentCopy) {
   const std::vector<std::string> values{"Switzerland", "Germany"};
   auto [metadata, test_column] = makeTestStringColumn(values);

   auto original = std::make_unique<StringInSet<StringColumn>>(
      &test_column,
      StringInSet<StringColumn>::Comparator::IN,
      std::unordered_set<std::string>{"Switzerland"}
   );

   auto copy = original->copy();

   ASSERT_EQ(original->toString(), copy->toString());
   ASSERT_EQ(original->match(0), copy->match(0));
   ASSERT_EQ(original->match(1), copy->match(1));
}

TEST(OperatorStringInSet, matchSingleValue) {
   const std::vector<std::string> values{"Apple", "Banana", "Cherry", "Apple", "Date"};
   auto [metadata, test_column] = makeTestStringColumn(values);
   const uint32_t row_count = values.size();

   auto under_test = std::make_unique<Selection>(
      std::make_unique<StringInSet<StringColumn>>(
         &test_column,
         StringInSet<StringColumn>::Comparator::IN,
         std::unordered_set<std::string>{"Apple"}
      ),
      row_count
   );

   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({0, 3}));
}

TEST(OperatorStringInSet, returnsCorrectTypeInfo) {
   const std::vector<std::string> values{"Switzerland"};
   auto [metadata, test_column] = makeTestStringColumn(values);
   const uint32_t row_count = values.size();

   const Selection under_test(
      std::make_unique<StringInSet<StringColumn>>(
         &test_column,
         StringInSet<StringColumn>::Comparator::IN,
         std::unordered_set<std::string>{"Switzerland"}
      ),
      row_count
   );

   ASSERT_EQ(under_test.type(), silo::query_engine::filter::operators::SELECTION);
}
