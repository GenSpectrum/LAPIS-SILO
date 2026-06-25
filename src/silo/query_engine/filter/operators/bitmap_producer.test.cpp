#include "silo/query_engine/filter/operators/bitmap_producer.h"

#include "silo/query_engine/copy_on_write_bitmap.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::CopyOnWriteBitmap;
using silo::query_engine::filter::operators::BitmapProducer;
using silo::storage::column::RowLayout;

TEST(OperatorBitmapProducer, evaluateShouldReturnCorrectValues) {
   const roaring::Roaring test_bitmap({1, 2, 3});
   const auto row_layout = RowLayout::of(5);

   const BitmapProducer under_test([&]() { return CopyOnWriteBitmap(&test_bitmap); }, row_layout);
   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring({1, 2, 3}));
}

TEST(OperatorBitmapProducer, evaluateShouldReturnCorrectValuesWhenNegated) {
   const roaring::Roaring test_bitmap({1, 2, 3});
   const auto row_layout = RowLayout::of(5);

   auto under_test = std::make_unique<BitmapProducer>(
      [&]() { return CopyOnWriteBitmap(&test_bitmap); }, row_layout
   );
   const auto negated = BitmapProducer::negate(std::move(under_test));

   ASSERT_EQ(negated->evaluate().getConstReference(), roaring::Roaring({0, 4}));
}

TEST(OperatorBitmapProducer, correctTypeInfo) {
   const roaring::Roaring test_bitmap({1, 2, 3});
   const auto row_layout = RowLayout::of(5);

   const BitmapProducer under_test([&]() { return CopyOnWriteBitmap(&test_bitmap); }, row_layout);
   ASSERT_EQ(under_test.type(), silo::query_engine::filter::operators::BITMAP_PRODUCER);
}

TEST(OperatorBitmapProducer, correctToString) {
   const roaring::Roaring test_bitmap({1, 2, 3});
   const auto row_layout = RowLayout::of(5);

   const BitmapProducer under_test([&]() { return CopyOnWriteBitmap(&test_bitmap); }, row_layout);
   ASSERT_EQ(under_test.toString(), "BitmapProducer");
}