#include "silo/query_engine/filter/operators/complement.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

#include "silo/query_engine/filter/operators/index_scan.h"

using silo::query_engine::CopyOnWriteBitmap;
using silo::query_engine::filter::operators::Complement;
using silo::query_engine::filter::operators::IndexScan;
using silo::storage::column::RowLayout;

TEST(OperatorComplement, evaluateShouldReturnCorrectValues) {
   const roaring::Roaring test_bitmap(roaring::Roaring({1, 2, 3}));
   const auto row_layout = RowLayout::of(5);

   const Complement under_test(
      std::make_unique<IndexScan>(CopyOnWriteBitmap{&test_bitmap}, row_layout), row_layout
   );
   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring({0, 4}));
}

TEST(OperatorComplement, evaluateShouldReturnCorrectValuesWhenEmptyInput) {
   const roaring::Roaring test_bitmap(roaring::Roaring({}));
   const auto row_layout = RowLayout::of(3);

   const Complement under_test(
      std::make_unique<IndexScan>(CopyOnWriteBitmap{&test_bitmap}, row_layout), row_layout
   );
   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring({0, 1, 2}));
}

TEST(OperatorComplement, evaluateShouldReturnCorrectValuesWhenEmptyDatabase) {
   const roaring::Roaring test_bitmap(roaring::Roaring({}));
   const auto row_layout = RowLayout::of(0);

   const Complement under_test(
      std::make_unique<IndexScan>(CopyOnWriteBitmap{&test_bitmap}, row_layout), row_layout
   );
   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring({}));
}

TEST(OperatorComplement, evaluateShouldReturnCorrectValuesWhenFullInput) {
   const roaring::Roaring test_bitmap(roaring::Roaring({0, 1, 2, 3}));
   const auto row_layout = RowLayout::of(4);

   const Complement under_test(
      std::make_unique<IndexScan>(CopyOnWriteBitmap{&test_bitmap}, row_layout), row_layout
   );
   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring({}));
}

TEST(OperatorComplement, evaluateShouldReturnCorrectValuesWhenSingleInput) {
   const roaring::Roaring test_bitmap(roaring::Roaring({1}));
   const auto row_layout = RowLayout::of(5);

   const Complement under_test(
      std::make_unique<IndexScan>(CopyOnWriteBitmap{&test_bitmap}, row_layout), row_layout
   );
   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring({0, 2, 3, 4}));
}

TEST(OperatorComplement, correctTypeInfo) {
   const roaring::Roaring test_bitmap({1, 2, 3});
   const auto row_layout = RowLayout::of(5);

   const Complement under_test(
      std::make_unique<IndexScan>(CopyOnWriteBitmap{&test_bitmap}, row_layout), row_layout
   );

   ASSERT_EQ(under_test.type(), silo::query_engine::filter::operators::COMPLEMENT);
}
