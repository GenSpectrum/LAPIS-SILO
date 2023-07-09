#include "silo/query_engine/operators/range_selection.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::operators::RangeSelection;
TEST(OperatorRangeSelection, evaluateShouldReturnCorrectValues) {
   std::vector<RangeSelection::Range> test_ranges(
      // NOLINTNEXTLINE(readability-magic-numbers)
      {{RangeSelection::Range{0, 2}, RangeSelection::Range{3, 5}}}
   );
   const uint32_t row_count = 8;

   const RangeSelection under_test(std::move(test_ranges), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 1, 3, 4}));
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({2, 5, 6, 7}));
}

TEST(OperatorRangeSelection, evaluateShouldReturnCorrectValuesEmptyDatabase) {
   std::vector<RangeSelection::Range> test_ranges;
   const uint32_t row_count = 0;

   const RangeSelection under_test(std::move(test_ranges), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring());
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring());
}

TEST(OperatorRangeSelection, evaluateShouldReturnCorrectValuesEmptyRanges) {
   std::vector<RangeSelection::Range> test_ranges(
      {{RangeSelection::Range{0, 0}, RangeSelection::Range{4, 4}}}
   );
   const uint32_t row_count = 9;

   const RangeSelection under_test(std::move(test_ranges), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring());
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({0, 1, 2, 3, 4, 5, 6, 7, 8}));
}

TEST(OperatorRangeSelection, evaluateShouldReturnCorrectValuesFullRange) {
   // NOLINTNEXTLINE(readability-magic-numbers)
   std::vector<RangeSelection::Range> test_ranges({{RangeSelection::Range{0, 8}}});
   const uint32_t row_count = 8;

   const RangeSelection under_test(std::move(test_ranges), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 1, 2, 3, 4, 5, 6, 7}));
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring());
}

TEST(OperatorRangeSelection, evaluateShouldReturnCorrectValuesMeetingRanges) {
   std::vector<RangeSelection::Range> test_ranges(
      {{RangeSelection::Range{0, 2}, RangeSelection::Range{2, 4}}}
   );
   const uint32_t row_count = 9;

   const RangeSelection under_test(std::move(test_ranges), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 1, 2, 3}));
   auto negated = under_test.negate();
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({4, 5, 6, 7, 8}));
}

TEST(OperatorRangeSelection, returnsCorrectTypeInfo) {
   std::vector<RangeSelection::Range> test_ranges(
      {{RangeSelection::Range{0, 2}, RangeSelection::Range{2, 4}}}
   );
   const uint32_t row_count = 8;

   const RangeSelection under_test(std::move(test_ranges), row_count);

   ASSERT_EQ(under_test.type(), silo::query_engine::operators::RANGE_SELECTION);
   auto negated = under_test.negate();
   ASSERT_EQ(negated->type(), silo::query_engine::operators::RANGE_SELECTION);
}
