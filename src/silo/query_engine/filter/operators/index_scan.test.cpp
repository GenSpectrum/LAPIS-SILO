#include "silo/query_engine/filter/operators/index_scan.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

#include "silo/query_engine/scalar_expressions/literal.h"

using silo::query_engine::CopyOnWriteBitmap;
using silo::query_engine::filter::operators::IndexScan;
using silo::query_engine::scalar_expressions::BoolLiteral;
using silo::storage::column::RowLayout;

TEST(OperatorIndexScan, evaluateShouldReturnCorrectValues) {
   const roaring::Roaring test_bitmap(roaring::Roaring({1, 3}));
   const IndexScan under_test(CopyOnWriteBitmap{&test_bitmap}, RowLayout::of(5));
   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring({1, 3}));
}

TEST(OperatorIndexScan, correctTypeInfo) {
   const roaring::Roaring test_bitmap({1, 2, 3});

   const IndexScan under_test(CopyOnWriteBitmap{&test_bitmap}, RowLayout::of(5));

   ASSERT_EQ(under_test.type(), silo::query_engine::filter::operators::INDEX_SCAN);
}

TEST(OperatorIndexScan, correctLogicalEquivalent) {
   const roaring::Roaring test_bitmap({1, 2, 3, 4, 5});
   const IndexScan under_test(
      std::make_unique<BoolLiteral>(true), CopyOnWriteBitmap{&test_bitmap}, RowLayout::of(5)
   );

   ASSERT_EQ(under_test.toString(), "IndexScan(Logical Equivalent: true, Cardinality: 5)");

   const roaring::Roaring test_bitmap2({});
   const IndexScan under_test2(
      std::make_unique<BoolLiteral>(false), CopyOnWriteBitmap{&test_bitmap}, RowLayout::of(5)
   );

   ASSERT_EQ(under_test2.toString(), "IndexScan(Logical Equivalent: false, Cardinality: 5)");
}
