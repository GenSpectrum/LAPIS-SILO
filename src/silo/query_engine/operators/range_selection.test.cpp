#include "silo/query_engine/operators/range_selection.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::operators::RangeSelection;

TEST(OperatorRangeSelection, evaluateShouldReturnCorrectValues) {
   std::vector<RangeSelection::Range> test_ranges(
      {{RangeSelection::Range(0, 2), RangeSelection::Range(3, 5)}}
   );

   RangeSelection under_test(std::move(test_ranges), 8);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 1, 3, 4}));
   under_test.negate();
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({2, 5, 6, 7}));
}

TEST(OperatorRangeSelection, evaluateShouldReturnCorrectValuesEmptyDatabase) {
   std::vector<RangeSelection::Range> test_ranges;

   RangeSelection under_test(std::move(test_ranges), 0);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring());
   under_test.negate();
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring());
}

TEST(OperatorRangeSelection, evaluateShouldReturnCorrectValuesEmptyRanges) {
   std::vector<RangeSelection::Range> test_ranges(
      {{RangeSelection::Range(0, 0), RangeSelection::Range(4, 4)}}
   );

   RangeSelection under_test(std::move(test_ranges), 8);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring());
   under_test.negate();
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 1, 2, 3, 4, 5, 6, 7}));
}

TEST(OperatorRangeSelection, evaluateShouldReturnCorrectValuesFullRange) {
   std::vector<RangeSelection::Range> test_ranges({{RangeSelection::Range(0, 8)}});

   RangeSelection under_test(std::move(test_ranges), 8);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 1, 2, 3, 4, 5, 6, 7}));
   under_test.negate();
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring());
}

TEST(OperatorRangeSelection, evaluateShouldReturnCorrectValuesMeetingRanges) {
   std::vector<RangeSelection::Range> test_ranges(
      {{RangeSelection::Range(0, 2), RangeSelection::Range(2, 4)}}
   );

   RangeSelection under_test(std::move(test_ranges), 8);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 1, 2, 3}));
   under_test.negate();
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({4, 5, 6, 7}));
}

TEST(OperatorRangeSelection, returnsCorrectTypeInfo) {
   std::vector<RangeSelection::Range> test_ranges(
      {{RangeSelection::Range(0, 2), RangeSelection::Range(2, 4)}}
   );

   RangeSelection under_test(std::move(test_ranges), 8);

   ASSERT_EQ(under_test.type(), silo::query_engine::operators::RANGE_SELECTION);
   under_test.negate();
   ASSERT_EQ(under_test.type(), silo::query_engine::operators::RANGE_SELECTION);
}
