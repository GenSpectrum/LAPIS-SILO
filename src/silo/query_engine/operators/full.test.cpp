#include "silo/query_engine/operators/full.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::operators::Full;

TEST(OperatorFull, containsCheckShouldReturnCorrectValues) {
   const Full under_test(5);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 1, 2, 3, 4}));
}

TEST(OperatorFull, containsCheckShouldReturnCorrectValuesWhenEmptyDatabase) {
   const Full under_test(0);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring());
}

TEST(OperatorFull, correctTypeInfo) {
   const Full under_test(5);

   ASSERT_EQ(under_test.type(), silo::query_engine::operators::FULL);
}
