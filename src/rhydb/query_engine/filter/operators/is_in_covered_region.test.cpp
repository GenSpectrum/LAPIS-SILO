#include "rhydb/query_engine/filter/operators/is_in_covered_region.h"

#include <gtest/gtest.h>
#include <memory>
#include <roaring/roaring.hh>
#include "rhydb/common/aligned_sequence.h"
#include "rhydb/query_engine/filter/operators/selection.h"

using rhydb::query_engine::filter::operators::IsInCoveredRegion;
using Comparator = IsInCoveredRegion::Comparator;
using rhydb::query_engine::filter::operators::Selection;
using rhydb::storage::column::RowId;
using rhydb::storage::column::RowLayout;

TEST(IsInCoveredRegion, containsCheckShouldReturnCorrectValues) {
   uint32_t global_row_id = 0;
   rhydb::storage::column::HorizontalCoverageIndex coverage_index;
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {1, 2, 3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {1, 3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {1, 2, 3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {4}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {1, 4}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {2, 4}}
   );
   auto under_test = std::make_unique<Selection>(
      std::make_unique<IsInCoveredRegion>(&coverage_index, 2, Comparator::IS_COVERED),
      RowLayout::of(coverage_index.chunkSize(0))
   );
   ASSERT_EQ(under_test->evaluate().toRoaring(), roaring::Roaring({1, 3, 4, 5, 6}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(negated->evaluate().toRoaring(), roaring::Roaring({0, 2, 7}));
}

TEST(IsInCoveredRegion, notContainsCheckShouldReturnCorrectValues) {
   uint32_t global_row_id = 0;
   rhydb::storage::column::HorizontalCoverageIndex coverage_index;
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {1, 2, 3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {1, 3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {1, 2, 3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {4}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {1, 4}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      rhydb::Coverage{.start = 0, .end = 5, .missing_positions = {2, 4}}
   );
   auto under_test = std::make_unique<Selection>(
      std::make_unique<IsInCoveredRegion>(&coverage_index, 2, Comparator::IS_NOT_COVERED),
      RowLayout::of(coverage_index.chunkSize(0))
   );
   ASSERT_EQ(under_test->evaluate().toRoaring(), roaring::Roaring({0, 2, 7}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(negated->evaluate().toRoaring(), roaring::Roaring({1, 3, 4, 5, 6}));
}
