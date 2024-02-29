#include "silo/query_engine/operators/bitmap_producer.h"

#include "silo/query_engine/operator_result.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::OperatorResult;
using silo::query_engine::operators::BitmapProducer;

TEST(OperatorBitmapProducer, evaluateShouldReturnCorrectValues) {
   const roaring::Roaring test_bitmap({1, 2, 3});
   const uint32_t row_count = 5;

   const BitmapProducer under_test([&]() { return OperatorResult(test_bitmap); }, row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 2, 3}));
}

TEST(OperatorBitmapProducer, evaluateShouldReturnCorrectValuesWhenNegated) {
   const roaring::Roaring test_bitmap({1, 2, 3});
   const uint32_t row_count = 5;

   auto under_test =
      std::make_unique<BitmapProducer>([&]() { return OperatorResult(test_bitmap); }, row_count);
   const auto negated = BitmapProducer::negate(std::move(under_test));

   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({0, 4}));
}

TEST(OperatorBitmapProducer, correctTypeInfo) {
   const roaring::Roaring test_bitmap({1, 2, 3});
   const uint32_t row_count = 5;

   const BitmapProducer under_test([&]() { return OperatorResult(test_bitmap); }, row_count);
   ASSERT_EQ(under_test.type(), silo::query_engine::operators::BITMAP_PRODUCER);
}

TEST(OperatorBitmapProducer, correctToString) {
   const roaring::Roaring test_bitmap({1, 2, 3});
   const uint32_t row_count = 5;

   const BitmapProducer under_test([&]() { return OperatorResult(test_bitmap); }, row_count);
   ASSERT_EQ(under_test.toString(), "BitmapProducer");
}