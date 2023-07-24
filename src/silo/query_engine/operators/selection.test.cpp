#include "silo/query_engine/operators/selection.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::operators::Comparator;
using silo::query_engine::operators::CompareToValueSelection;
using silo::query_engine::operators::Selection;

TEST(OperatorSelection, equalsShouldReturnCorrectValues) {
   const std::vector<int32_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   const uint32_t row_count = test_column.size();

   const Selection under_test(

      std::make_unique<CompareToValueSelection<int32_t>>(test_column, Comparator::EQUALS, 1),
      row_count
   );

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 5, 6, 7, 8, 9}));
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({0, 2, 3, 4}));
}

TEST(OperatorSelection, notEqualsShouldReturnCorrectValues) {
   const std::vector<int32_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   const uint32_t row_count = test_column.size();

   const Selection under_test(

      std::make_unique<CompareToValueSelection<int32_t>>(test_column, Comparator::NOT_EQUALS, 1),
      row_count
   );

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 2, 3, 4}));
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({1, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, lessShouldReturnCorrectValues) {
   const std::vector<int32_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   const uint32_t row_count = test_column.size();

   const Selection under_test(
      std::make_unique<CompareToValueSelection<int32_t>>(test_column, Comparator::LESS, 1),
      row_count
   );

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0}));
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({1, 2, 3, 4, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, lessOrEqualsShouldReturnCorrectValues) {
   const std::vector<int32_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   const uint32_t row_count = test_column.size();

   const Selection under_test(
      std::make_unique<CompareToValueSelection<int32_t>>(
         test_column, Comparator::LESS_OR_EQUALS, 1
      ),
      row_count
   );

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 1, 5, 6, 7, 8, 9}));
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({2, 3, 4}));
}

TEST(OperatorSelection, higherOrEqualsShouldReturnCorrectValues) {
   const std::vector<int32_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   const uint32_t row_count = test_column.size();

   const Selection under_test(

      std::make_unique<CompareToValueSelection<int32_t>>(
         test_column, Comparator::HIGHER_OR_EQUALS, 1
      ),
      row_count
   );

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 2, 3, 4, 5, 6, 7, 8, 9}));
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({0}));
}

TEST(OperatorSelection, higherShouldReturnCorrectValues) {
   const std::vector<int32_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   const uint32_t row_count = test_column.size();

   const Selection under_test(

      std::make_unique<CompareToValueSelection<int32_t>>(test_column, Comparator::HIGHER, 1),
      row_count
   );

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({2, 3, 4}));
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({0, 1, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, correctWithNegativeNumbers) {
   const std::vector<int32_t> test_column({{0, -1, 4, 4, 4, -1, -1, -1, -1, -1}});
   const uint32_t row_count = test_column.size();

   const Selection under_test(

      std::make_unique<CompareToValueSelection<int32_t>>(test_column, Comparator::EQUALS, -1),
      row_count
   );

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, returnsCorrectTypeInfo) {
   const std::vector<int32_t> test_column({{0, -1, 4, 4, 4, -1, -1, -1, -1, -1}});
   const uint32_t row_count = test_column.size();

   const Selection under_test(

      std::make_unique<CompareToValueSelection<int32_t>>(test_column, Comparator::EQUALS, -1),
      row_count
   );

   ASSERT_EQ(under_test.type(), silo::query_engine::operators::SELECTION);
}
