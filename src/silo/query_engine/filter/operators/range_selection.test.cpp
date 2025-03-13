#include "silo/query_engine/filter/operators/range_selection.h"

#include <gtest/gtest.h>

#include "external/roaring_include_wrapper.h"

using silo::query_engine::filter::operators::RangeSelection;
TEST(OperatorRangeSelection, evaluateShouldReturnCorrectValues) {
   std::vector<RangeSelection::Range> test_ranges(
      // NOLINTNEXTLINE(readability-magic-numbers)
      {{RangeSelection::Range{0, 2}, RangeSelection::Range{3, 5}}}
   );
   const uint32_t row_count = 8;

   auto under_test = std::make_unique<RangeSelection>(std::move(test_ranges), row_count);
   ASSERT_EQ(*under_test->evaluate(), roaring::Roaring({0, 1, 3, 4}));
   auto negated = RangeSelection::negate(std::move(under_test));
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({2, 5, 6, 7}));
}

TEST(OperatorRangeSelection, evaluateShouldReturnCorrectValuesEmptyDatabase) {
   std::vector<RangeSelection::Range> test_ranges;
   const uint32_t row_count = 0;

   auto under_test = std::make_unique<RangeSelection>(std::move(test_ranges), row_count);
   ASSERT_EQ(*under_test->evaluate(), roaring::Roaring());
   auto negated = RangeSelection::negate(std::move(under_test));
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring());
}

TEST(OperatorRangeSelection, evaluateShouldReturnCorrectValuesEmptyRanges) {
   std::vector<RangeSelection::Range> test_ranges(
      {{RangeSelection::Range{0, 0}, RangeSelection::Range{4, 4}}}
   );
   const uint32_t row_count = 9;

   auto under_test = std::make_unique<RangeSelection>(std::move(test_ranges), row_count);
   ASSERT_EQ(*under_test->evaluate(), roaring::Roaring());
   auto negated = RangeSelection::negate(std::move(under_test));
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({0, 1, 2, 3, 4, 5, 6, 7, 8}));
}

TEST(OperatorRangeSelection, evaluateShouldReturnCorrectValuesFullRange) {
   // NOLINTNEXTLINE(readability-magic-numbers)
   std::vector<RangeSelection::Range> test_ranges({{RangeSelection::Range{0, 8}}});
   const uint32_t row_count = 8;

   auto under_test = std::make_unique<RangeSelection>(std::move(test_ranges), row_count);
   ASSERT_EQ(*under_test->evaluate(), roaring::Roaring({0, 1, 2, 3, 4, 5, 6, 7}));
   auto negated = RangeSelection::negate(std::move(under_test));
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring());
}

TEST(OperatorRangeSelection, evaluateShouldReturnCorrectValuesMeetingRanges) {
   std::vector<RangeSelection::Range> test_ranges(
      {{RangeSelection::Range{0, 2}, RangeSelection::Range{2, 4}}}
   );
   const uint32_t row_count = 9;

   auto under_test = std::make_unique<RangeSelection>(std::move(test_ranges), row_count);
   ASSERT_EQ(*under_test->evaluate(), roaring::Roaring({0, 1, 2, 3}));
   auto negated = RangeSelection::negate(std::move(under_test));
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({4, 5, 6, 7, 8}));
}

TEST(OperatorRangeSelection, returnsCorrectTypeInfo) {
   std::vector<RangeSelection::Range> test_ranges(
      {{RangeSelection::Range{0, 2}, RangeSelection::Range{2, 4}}}
   );
   const uint32_t row_count = 8;

   auto under_test = std::make_unique<RangeSelection>(std::move(test_ranges), row_count);

   ASSERT_EQ(under_test->type(), silo::query_engine::filter::operators::RANGE_SELECTION);
   auto negated = RangeSelection::negate(std::move(under_test));
   ASSERT_EQ(negated->type(), silo::query_engine::filter::operators::RANGE_SELECTION);
}
