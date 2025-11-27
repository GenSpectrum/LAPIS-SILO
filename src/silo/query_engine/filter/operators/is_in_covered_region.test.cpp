#include "silo/query_engine/filter/operators/is_in_covered_region.h"

#include <gtest/gtest.h>
#include <memory>
#include <roaring/roaring.hh>
#include "silo/query_engine/filter/operators/selection.h"

using silo::query_engine::filter::operators::IsInCoveredRegion;
using Comparator = silo::query_engine::filter::operators::IsInCoveredRegion::Comparator;
using silo::query_engine::filter::operators::Selection;

TEST(IsInCoveredRegion, containsCheckShouldReturnCorrectValues) {
   silo::storage::column::HorizontalCoverageIndex coverage_index;
   coverage_index.insertCoverage(0, 5, {1, 2, 3});
   coverage_index.insertCoverage(0, 5, {1, 3});
   coverage_index.insertCoverage(0, 5, {1, 2, 3});
   coverage_index.insertCoverage(0, 5, {});
   coverage_index.insertCoverage(0, 5, {3});
   coverage_index.insertCoverage(0, 5, {4});
   coverage_index.insertCoverage(0, 5, {1, 4});
   coverage_index.insertCoverage(0, 5, {2, 4});
   auto under_test = std::make_unique<Selection>(
      std::make_unique<IsInCoveredRegion>(&coverage_index, 2, Comparator::IS_COVERED),
      coverage_index.start_end.size()
   );
   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({1, 3, 4, 5, 6}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(negated->evaluate().getConstReference(), roaring::Roaring({0, 2, 7}));
}

TEST(IsInCoveredRegion, notContainsCheckShouldReturnCorrectValues) {
   silo::storage::column::HorizontalCoverageIndex coverage_index;
   coverage_index.insertCoverage(0, 5, {1, 2, 3});
   coverage_index.insertCoverage(0, 5, {1, 3});
   coverage_index.insertCoverage(0, 5, {1, 2, 3});
   coverage_index.insertCoverage(0, 5, {});
   coverage_index.insertCoverage(0, 5, {3});
   coverage_index.insertCoverage(0, 5, {4});
   coverage_index.insertCoverage(0, 5, {1, 4});
   coverage_index.insertCoverage(0, 5, {2, 4});
   auto under_test = std::make_unique<Selection>(
      std::make_unique<IsInCoveredRegion>(
         &coverage_index,
         2,
         silo::query_engine::filter::operators::IsInCoveredRegion::Comparator::IS_NOT_COVERED
      ),
      coverage_index.start_end.size()
   );
   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({0, 2, 7}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(negated->evaluate().getConstReference(), roaring::Roaring({1, 3, 4, 5, 6}));
}
