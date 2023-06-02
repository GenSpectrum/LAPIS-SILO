#include "silo/query_engine/operators/selection.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::operators::Selection;

TEST(OperatorSelection, equalsShouldReturnCorrectValues) {
   const std::vector<uint64_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   const uint32_t row_count = 13;

   const Selection<uint64_t> under_test(
      test_column, Selection<uint64_t>::Comparator::EQUALS, 1, row_count
   );

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 5, 6, 7, 8, 9}));
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({0, 2, 3, 4}));
}

TEST(OperatorSelection, notEqualsShouldReturnCorrectValues) {
   const std::vector<uint64_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   const uint32_t row_count = 12;

   const Selection<uint64_t> under_test(
      test_column, Selection<uint64_t>::Comparator::NOT_EQUALS, 1, row_count
   );

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 2, 3, 4}));
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({1, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, lessShouldReturnCorrectValues) {
   const std::vector<uint64_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   const uint32_t row_count = 15;

   const Selection<uint64_t> under_test(
      test_column, Selection<uint64_t>::Comparator::LESS, 1, row_count
   );

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0}));
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({1, 2, 3, 4, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, lessOrEqualsShouldReturnCorrectValues) {
   const std::vector<uint64_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   const uint32_t row_count = 13;

   const Selection<uint64_t> under_test(
      test_column, Selection<uint64_t>::Comparator::LESS_OR_EQUALS, 1, row_count
   );

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 1, 5, 6, 7, 8, 9}));
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({2, 3, 4}));
}

TEST(OperatorSelection, higherOrEqualsShouldReturnCorrectValues) {
   const std::vector<uint64_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   const uint32_t row_count = 13;

   const Selection<uint64_t> under_test(
      test_column, Selection<uint64_t>::Comparator::HIGHER_OR_EQUALS, 1, row_count
   );

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 2, 3, 4, 5, 6, 7, 8, 9}));
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({0}));
}

TEST(OperatorSelection, higherShouldReturnCorrectValues) {
   const std::vector<uint64_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   const uint32_t row_count = 13;

   const Selection<uint64_t> under_test(
      test_column, Selection<uint64_t>::Comparator::HIGHER, 1, row_count
   );

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({2, 3, 4}));
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({0, 1, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, returnsCorrectTypeInfo) {
   const std::vector<uint64_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   const uint32_t row_count = 13;

   const Selection<uint64_t> under_test(
      test_column, Selection<uint64_t>::Comparator::EQUALS, 1, row_count
   );

   ASSERT_EQ(under_test.type(), silo::query_engine::operators::SELECTION);
}
