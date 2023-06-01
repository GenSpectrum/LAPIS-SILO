#include "silo/query_engine/operators/selection.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::operators::Selection;

TEST(OperatorSelection, equalsShouldReturnCorrectValues) {
   std::vector<uint64_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});

   Selection<uint64_t> under_test(test_column, Selection<uint64_t>::Comparator::EQUALS, 1);

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 5, 6, 7, 8, 9}));
   under_test.negate();
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 2, 3, 4}));
}

TEST(OperatorSelection, notEqualsShouldReturnCorrectValues) {
   std::vector<uint64_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});

   Selection<uint64_t> under_test(test_column, Selection<uint64_t>::Comparator::NOT_EQUALS, 1);

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 2, 3, 4}));
   under_test.negate();
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, lessShouldReturnCorrectValues) {
   std::vector<uint64_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});

   Selection<uint64_t> under_test(test_column, Selection<uint64_t>::Comparator::LESS, 1);

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0}));
   under_test.negate();
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 2, 3, 4, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, lessOrEqualsShouldReturnCorrectValues) {
   std::vector<uint64_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});

   Selection<uint64_t> under_test(test_column, Selection<uint64_t>::Comparator::LESS_OR_EQUALS, 1);

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 1, 5, 6, 7, 8, 9}));
   under_test.negate();
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({2, 3, 4}));
}

TEST(OperatorSelection, higherOrEqualsShouldReturnCorrectValues) {
   std::vector<uint64_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});

   Selection<uint64_t> under_test(
      test_column, Selection<uint64_t>::Comparator::HIGHER_OR_EQUALS, 1
   );

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 2, 3, 4, 5, 6, 7, 8, 9}));
   under_test.negate();
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0}));
}

TEST(OperatorSelection, higherShouldReturnCorrectValues) {
   std::vector<uint64_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});

   Selection<uint64_t> under_test(test_column, Selection<uint64_t>::Comparator::HIGHER, 1);

   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({2, 3, 4}));
   under_test.negate();
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 1, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, returnsCorrectTypeInfo) {
   std::vector<uint64_t> test_column({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});

   const Selection<uint64_t> under_test(test_column, Selection<uint64_t>::Comparator::EQUALS, 1);

   ASSERT_EQ(under_test.type(), silo::query_engine::operators::SELECTION);
}
