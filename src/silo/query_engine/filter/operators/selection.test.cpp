#include "silo/query_engine/filter/operators/selection.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

#include "silo/storage/column/int_column.h"

using silo::query_engine::filter::operators::Comparator;
using silo::query_engine::filter::operators::CompareToValueSelection;
using silo::query_engine::filter::operators::Selection;
using silo::storage::column::ColumnMetadata;
using silo::storage::column::IntColumnPartition;

namespace {

std::pair<std::shared_ptr<ColumnMetadata>, IntColumnPartition> makeTestColumn(
   const std::vector<int32_t> values
) {
   auto metadata = std::make_shared<ColumnMetadata>("test");
   IntColumnPartition test_column{metadata.get()};
   for (auto value : values) {
      test_column.insert(value);
   }
   return {metadata, test_column};
}

}  // namespace

TEST(OperatorSelection, equalsShouldReturnCorrectValues) {
   const std::vector<int32_t> values({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   auto [metadata, test_column] = makeTestColumn(values);
   const uint32_t row_count = values.size();

   auto under_test = std::make_unique<Selection>(
      std::make_unique<CompareToValueSelection<IntColumnPartition>>(
         test_column, Comparator::EQUALS, 1
      ),
      row_count
   );

   ASSERT_EQ(*under_test->evaluate(), roaring::Roaring({1, 5, 6, 7, 8, 9}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({0, 2, 3, 4}));
}

TEST(OperatorSelection, notEqualsShouldReturnCorrectValues) {
   const std::vector<int32_t> values({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   auto [metadata, test_column] = makeTestColumn(values);
   const uint32_t row_count = values.size();

   auto under_test = std::make_unique<Selection>(
      std::make_unique<CompareToValueSelection<IntColumnPartition>>(
         test_column, Comparator::NOT_EQUALS, 1
      ),
      row_count
   );

   ASSERT_EQ(*under_test->evaluate(), roaring::Roaring({0, 2, 3, 4}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({1, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, lessShouldReturnCorrectValues) {
   const std::vector<int32_t> values({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   auto [metadata, test_column] = makeTestColumn(values);
   const uint32_t row_count = values.size();

   auto under_test = std::make_unique<Selection>(
      std::make_unique<CompareToValueSelection<IntColumnPartition>>(
         test_column, Comparator::LESS, 1
      ),
      row_count
   );

   ASSERT_EQ(*under_test->evaluate(), roaring::Roaring({0}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({1, 2, 3, 4, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, lessOrEqualsShouldReturnCorrectValues) {
   const std::vector<int32_t> values({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   auto [metadata, test_column] = makeTestColumn(values);
   const uint32_t row_count = values.size();

   auto under_test = std::make_unique<Selection>(
      std::make_unique<CompareToValueSelection<IntColumnPartition>>(
         test_column, Comparator::LESS_OR_EQUALS, 1
      ),
      row_count
   );

   ASSERT_EQ(*under_test->evaluate(), roaring::Roaring({0, 1, 5, 6, 7, 8, 9}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({2, 3, 4}));
}

TEST(OperatorSelection, higherOrEqualsShouldReturnCorrectValues) {
   const std::vector<int32_t> values({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   auto [metadata, test_column] = makeTestColumn(values);
   const uint32_t row_count = values.size();

   auto under_test = std::make_unique<Selection>(
      std::make_unique<CompareToValueSelection<IntColumnPartition>>(
         test_column, Comparator::HIGHER_OR_EQUALS, 1
      ),
      row_count
   );

   ASSERT_EQ(*under_test->evaluate(), roaring::Roaring({1, 2, 3, 4, 5, 6, 7, 8, 9}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({0}));
}

TEST(OperatorSelection, higherShouldReturnCorrectValues) {
   const std::vector<int32_t> values({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   auto [metadata, test_column] = makeTestColumn(values);
   const uint32_t row_count = values.size();

   auto under_test = std::make_unique<Selection>(
      std::make_unique<CompareToValueSelection<IntColumnPartition>>(
         test_column, Comparator::HIGHER, 1
      ),
      row_count
   );

   ASSERT_EQ(*under_test->evaluate(), roaring::Roaring({2, 3, 4}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({0, 1, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, correctWithNegativeNumbers) {
   const std::vector<int32_t> values({{0, -1, 4, 4, 4, -1, -1, -1, -1, -1}});
   auto [metadata, test_column] = makeTestColumn(values);
   const uint32_t row_count = values.size();

   const Selection under_test(
      std::make_unique<CompareToValueSelection<IntColumnPartition>>(
         test_column, Comparator::EQUALS, -1
      ),
      row_count
   );

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, returnsCorrectTypeInfo) {
   std::vector<int32_t> values{{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}};
   auto [metadata, test_column] = makeTestColumn(values);
   const uint32_t row_count = values.size();

   const Selection under_test(
      std::make_unique<CompareToValueSelection<IntColumnPartition>>(
         test_column, Comparator::EQUALS, -1
      ),
      row_count
   );

   ASSERT_EQ(under_test.type(), silo::query_engine::filter::operators::SELECTION);
}
