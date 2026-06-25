#include "silo/query_engine/filter/operators/full.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::filter::operators::Full;
using silo::storage::column::RowLayout;

TEST(OperatorFull, containsCheckShouldReturnCorrectValues) {
   const Full under_test(RowLayout::of(5));
   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring({0, 1, 2, 3, 4}));
}

TEST(OperatorFull, containsCheckShouldReturnCorrectValuesWhenEmptyDatabase) {
   const Full under_test(RowLayout::of(0));
   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring());
}

TEST(OperatorFull, correctTypeInfo) {
   const Full under_test(RowLayout::of(5));

   ASSERT_EQ(under_test.type(), silo::query_engine::filter::operators::FULL);
}
