#include "silo/query_engine/filter/operators/index_scan.h"

#include <gtest/gtest.h>

#include "external/roaring_include_wrapper.h"
#include "silo/query_engine/filter/expressions/false.h"
#include "silo/query_engine/filter/expressions/true.h"

using silo::query_engine::filter::expressions::False;
using silo::query_engine::filter::expressions::True;
using silo::query_engine::filter::operators::IndexScan;

TEST(OperatorIndexScan, evaluateShouldReturnCorrectValues) {
   const roaring::Roaring test_bitmap(roaring::Roaring({1, 3}));
   const IndexScan under_test(&test_bitmap, 5);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 3}));
}

TEST(OperatorIndexScan, correctTypeInfo) {
   const roaring::Roaring test_bitmap({1, 2, 3});

   const IndexScan under_test(&test_bitmap, 5);

   ASSERT_EQ(under_test.type(), silo::query_engine::filter::operators::INDEX_SCAN);
}

TEST(OperatorIndexScan, correctLogicalEquivalent) {
   const roaring::Roaring test_bitmap({1, 2, 3, 4, 5});
   const IndexScan under_test(std::make_unique<True>(), &test_bitmap, 5);

   ASSERT_EQ(under_test.toString(), "IndexScan(Logical Equivalent: True, Cardinality: 5)");

   const roaring::Roaring test_bitmap2({});
   const IndexScan under_test2(std::make_unique<False>(), &test_bitmap, 5);

   ASSERT_EQ(under_test2.toString(), "IndexScan(Logical Equivalent: False, Cardinality: 5)");
}
