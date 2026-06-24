#include "silo/query_engine/filter/operators/empty.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::filter::operators::Empty;
using silo::storage::column::RowLayout;

TEST(OperatorEmpty, evaluateShouldReturnNoValues) {
   const Empty under_test(RowLayout::of(1));
   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring());
}

TEST(OperatorEmpty, correctTypeInfo) {
   const Empty under_test(RowLayout::of(1));

   ASSERT_EQ(under_test.type(), silo::query_engine::filter::operators::EMPTY);
}
