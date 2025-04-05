#include "silo/query_engine/filter/operators/empty.h"

#include <gtest/gtest.h>

#include "external/roaring_include_wrapper.h"

using silo::query_engine::filter::operators::Empty;

TEST(OperatorEmpty, evaluateShouldReturnNoValues) {
   const Empty under_test(1);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring());
}

TEST(OperatorEmpty, correctTypeInfo) {
   const Empty under_test(1);

   ASSERT_EQ(under_test.type(), silo::query_engine::filter::operators::EMPTY);
}
