#include "silo/query_engine/filter/operators/full.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using rhydb::query_engine::filter::operators::Full;
using rhydb::storage::column::RowLayout;

TEST(OperatorFull, containsCheckShouldReturnCorrectValues) {
   const Full under_test(RowLayout::of(5));
   ASSERT_EQ(under_test.evaluate().toRoaring(), roaring::Roaring({0, 1, 2, 3, 4}));
}

TEST(OperatorFull, containsCheckShouldReturnCorrectValuesWhenEmptyDatabase) {
   const Full under_test(RowLayout::of());
   ASSERT_EQ(under_test.evaluate().toRoaring(), roaring::Roaring());
}

TEST(OperatorFull, correctTypeInfo) {
   const Full under_test(RowLayout::of(5));

   ASSERT_EQ(under_test.type(), rhydb::query_engine::filter::operators::FULL);
}
