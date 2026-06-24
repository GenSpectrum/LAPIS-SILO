#include "silo/query_engine/filter/operators/is_in_covered_region.h"

#include <gtest/gtest.h>
#include <memory>
#include <roaring/roaring.hh>
#include "silo/common/aligned_sequence.h"
#include "silo/query_engine/filter/operators/selection.h"

using silo::query_engine::filter::operators::IsInCoveredRegion;
using Comparator = IsInCoveredRegion::Comparator;
using silo::query_engine::filter::operators::Selection;
using silo::storage::column::RowId;
using silo::storage::column::RowLayout;

TEST(IsInCoveredRegion, containsCheckShouldReturnCorrectValues) {
   uint32_t global_row_id = 0;
   silo::storage::column::HorizontalCoverageIndex coverage_index;
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {1, 2, 3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {1, 3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {1, 2, 3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {4}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {1, 4}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {2, 4}}
   );
   auto under_test = std::make_unique<Selection>(
      std::make_unique<IsInCoveredRegion>(&coverage_index, 2, Comparator::IS_COVERED),
      RowLayout::of(coverage_index.start_end.at(0).size())
   );
   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({1, 3, 4, 5, 6}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(negated->evaluate().getConstReference(), roaring::Roaring({0, 2, 7}));
}

TEST(IsInCoveredRegion, notContainsCheckShouldReturnCorrectValues) {
   uint32_t global_row_id = 0;
   silo::storage::column::HorizontalCoverageIndex coverage_index;
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {1, 2, 3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {1, 3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {1, 2, 3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {3}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {4}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {1, 4}}
   );
   coverage_index.insertCoverage(
      RowId::fromGlobal(global_row_id++),
      silo::Coverage{.start = 0, .end = 5, .missing_positions = {2, 4}}
   );
   auto under_test = std::make_unique<Selection>(
      std::make_unique<IsInCoveredRegion>(&coverage_index, 2, Comparator::IS_NOT_COVERED),
      RowLayout::of(coverage_index.start_end.at(0).size())
   );
   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({0, 2, 7}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(negated->evaluate().getConstReference(), roaring::Roaring({1, 3, 4, 5, 6}));
}
