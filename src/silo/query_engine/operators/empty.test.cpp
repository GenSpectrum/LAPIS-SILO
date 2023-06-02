#include "silo/query_engine/operators/empty.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::operators::Empty;

TEST(OperatorEmpty, evaluateShouldReturnNoValues) {
   const Empty under_test(1);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring());
}

TEST(OperatorEmpty, correctTypeInfo) {
   const Empty under_test(1);

   ASSERT_EQ(under_test.type(), silo::query_engine::operators::EMPTY);
}
