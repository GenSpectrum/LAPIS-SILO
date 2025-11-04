#include "silo/query_engine/filter/operators/is_in_covered_region.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::filter::operators::IsInCoveredRegion;

TEST(IsInCoveredRegion, containsCheckShouldReturnCorrectValues) {
   const std::vector<std::pair<size_t, size_t>> start_ends{
      {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}
   };
   const std::map<size_t, roaring::Roaring> test_bitmaps({{
      {0, roaring::Roaring({1, 2, 3})},
      {1, roaring::Roaring({1, 3})},
      {2, roaring::Roaring({1, 2, 3})},
      {3, roaring::Roaring({})},
      {4, roaring::Roaring({3})},
      {5, roaring::Roaring({4})},
      {6, roaring::Roaring({1, 4})},
      {7, roaring::Roaring({2, 4})},
   }});
   auto under_test = std::make_unique<IsInCoveredRegion>(
      &start_ends, &test_bitmaps, test_bitmaps.size(), IsInCoveredRegion::Comparator::NOT_COVERED, 2
   );
   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({0, 2, 7}));
   auto negated = IsInCoveredRegion::negate(std::move(under_test));
   ASSERT_EQ(negated->evaluate().getConstReference(), roaring::Roaring({1, 3, 4, 5, 6}));
}

TEST(IsInCoveredRegion, notContainsCheckShouldReturnCorrectValues) {
   const std::vector<std::pair<size_t, size_t>> start_ends{
      {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}
   };
   const std::map<size_t, roaring::Roaring> test_bitmaps({{
      {0, roaring::Roaring({1, 2, 3})},
      {1, roaring::Roaring({1, 3})},
      {2, roaring::Roaring({1, 2, 3})},
      {3, roaring::Roaring({})},
      {4, roaring::Roaring({3})},
      {5, roaring::Roaring({4})},
      {6, roaring::Roaring({1, 4})},
      {7, roaring::Roaring({2, 4})},
   }});
   auto under_test = std::make_unique<IsInCoveredRegion>(
      &start_ends, &test_bitmaps, test_bitmaps.size(), IsInCoveredRegion::Comparator::COVERED, 2
   );
   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({1, 3, 4, 5, 6}));
   auto negated = IsInCoveredRegion::negate(std::move(under_test));
   ASSERT_EQ(negated->evaluate().getConstReference(), roaring::Roaring({0, 2, 7}));
}

TEST(IsInCoveredRegion, containsCheckOutsideRegion) {
   const std::vector<std::pair<size_t, size_t>> start_ends{
      {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}
   };
   const std::map<size_t, roaring::Roaring> test_bitmaps({{
      {0, roaring::Roaring({1, 2, 3})},
      {1, roaring::Roaring({1, 3})},
      {2, roaring::Roaring({1, 2, 3})},
      {3, roaring::Roaring({})},
      {4, roaring::Roaring({3})},
      {5, roaring::Roaring({4})},
      {6, roaring::Roaring({1, 4})},
      {7, roaring::Roaring({2, 4})},
   }});
   auto under_test = std::make_unique<IsInCoveredRegion>(
      &start_ends, &test_bitmaps, test_bitmaps.size(), IsInCoveredRegion::Comparator::COVERED, 7
   );
   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({}));
   auto negated = IsInCoveredRegion::negate(std::move(under_test));
   ASSERT_EQ(negated->evaluate().getConstReference(), roaring::Roaring({0, 1, 2, 3, 4, 5, 6, 7}));
}

TEST(IsInCoveredRegion, correctTypeInfo) {
   const std::vector<std::pair<size_t, size_t>> start_ends{
      {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}, {0, 5}
   };
   const std::map<size_t, roaring::Roaring> test_bitmaps({{
      {0, roaring::Roaring({1, 2, 3})},
      {1, roaring::Roaring({1, 3})},
      {2, roaring::Roaring({1, 2, 3})},
      {3, roaring::Roaring({})},
      {4, roaring::Roaring({3})},
      {5, roaring::Roaring({4})},
      {6, roaring::Roaring({1, 4})},
      {7, roaring::Roaring({2, 4})},
   }});
   auto under_test = std::make_unique<IsInCoveredRegion>(
      &start_ends, &test_bitmaps, test_bitmaps.size(), IsInCoveredRegion::Comparator::COVERED, 2
   );

   ASSERT_EQ(under_test->type(), silo::query_engine::filter::operators::IS_IN_COVERED_REGION);
   auto negated = IsInCoveredRegion::negate(std::move(under_test));
   ASSERT_EQ(negated->type(), silo::query_engine::filter::operators::IS_IN_COVERED_REGION);
}
