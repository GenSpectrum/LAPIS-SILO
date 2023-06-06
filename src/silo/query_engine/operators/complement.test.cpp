#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/index_scan.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::operators::Complement;
using silo::query_engine::operators::IndexScan;

TEST(OperatorComplement, evaluateShouldReturnCorrectValues) {
   const roaring::Roaring test_bitmap(roaring::Roaring({1, 2, 3}));
   const uint32_t row_count = 5;

   const Complement under_test(std::make_unique<IndexScan>(&test_bitmap, row_count), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 4}));
}

TEST(OperatorComplement, evaluateShouldReturnCorrectValuesWhenEmptyInput) {
   const roaring::Roaring test_bitmap(roaring::Roaring({}));
   const uint32_t row_count = 3;

   const Complement under_test(std::make_unique<IndexScan>(&test_bitmap, row_count), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 1, 2}));
}

TEST(OperatorComplement, evaluateShouldReturnCorrectValuesWhenEmptyDatabase) {
   const roaring::Roaring test_bitmap(roaring::Roaring({}));
   const uint32_t row_count = 0;

   const Complement under_test(std::make_unique<IndexScan>(&test_bitmap, row_count), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({}));
}

TEST(OperatorComplement, evaluateShouldReturnCorrectValuesWhenFullInput) {
   const roaring::Roaring test_bitmap(roaring::Roaring({0, 1, 2, 3}));
   const uint32_t row_count = 4;

   const Complement under_test(std::make_unique<IndexScan>(&test_bitmap, row_count), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({}));
}

TEST(OperatorComplement, evaluateShouldReturnCorrectValuesWhenSingleInput) {
   const roaring::Roaring test_bitmap(roaring::Roaring({1}));
   const uint32_t row_count = 5;

   const Complement under_test(std::make_unique<IndexScan>(&test_bitmap, row_count), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 2, 3, 4}));
}

TEST(OperatorComplement, correctTypeInfo) {
   const roaring::Roaring test_bitmap({1, 2, 3});
   const uint32_t row_count = 5;

   const Complement under_test(std::make_unique<IndexScan>(&test_bitmap, row_count), row_count);

   ASSERT_EQ(under_test.type(), silo::query_engine::operators::COMPLEMENT);
}
