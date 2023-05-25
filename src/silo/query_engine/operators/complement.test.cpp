#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/index_scan.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::operators::Complement;
using silo::query_engine::operators::IndexScan;

TEST(OperatorComplement, evaluateShouldReturnCorrectValues) {
   roaring::Roaring test_bitmap(roaring::Roaring({1, 2, 3}));
   Complement under_test(std::make_unique<IndexScan>(&test_bitmap), 5);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 4}));
}

TEST(OperatorComplement, evaluateShouldReturnCorrectValuesWhenEmptyInput) {
   roaring::Roaring test_bitmap(roaring::Roaring({}));
   Complement under_test(std::make_unique<IndexScan>(&test_bitmap), 3);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 1, 2}));
}

TEST(OperatorComplement, evaluateShouldReturnCorrectValuesWhenEmptyDatabase) {
   roaring::Roaring test_bitmap(roaring::Roaring({}));
   Complement under_test(std::make_unique<IndexScan>(&test_bitmap), 0);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({}));
}

TEST(OperatorComplement, evaluateShouldReturnCorrectValuesWhenFullInput) {
   roaring::Roaring test_bitmap(roaring::Roaring({0, 1, 2, 3}));
   Complement under_test(std::make_unique<IndexScan>(&test_bitmap), 4);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({}));
}

TEST(OperatorComplement, evaluateShouldReturnCorrectValuesWhenSingleInput) {
   roaring::Roaring test_bitmap(roaring::Roaring({1}));
   Complement under_test(std::make_unique<IndexScan>(&test_bitmap), 5);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 2, 3, 4}));
}

TEST(OperatorComplement, correctTypeInfo) {
   roaring::Roaring test_bitmap({1, 2, 3});

   Complement under_test(std::make_unique<IndexScan>(&test_bitmap), 5);

   ASSERT_EQ(under_test.type(), silo::query_engine::operators::COMPLEMENT);
}
