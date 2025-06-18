#include "silo/query_engine/filter/operators/union.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/query_compilation_exception.h"

using silo::query_engine::filter::operators::IndexScan;
using silo::query_engine::filter::operators::Operator;
using silo::query_engine::filter::operators::Union;

using silo::query_engine::filter::operators::OperatorVector;

namespace {
OperatorVector generateTestInput(const std::vector<roaring::Roaring>& bitmaps, uint32_t row_count) {
   OperatorVector result;
   std::ranges::transform(bitmaps, std::back_inserter(result), [&](const auto& bitmap) {
      return std::make_unique<IndexScan>(&bitmap, row_count);
   });
   return result;
}
}  // namespace

TEST(OperatorUnion, evaluatesCorrectOnEmptyInput) {
   OperatorVector input;
   const uint32_t row_count = 5;

   const Union under_test(std::move(input), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring());
}

TEST(OperatorUnion, evaluatesCorrectOnOneInput) {
   const std::vector<roaring::Roaring> test_bitmaps({{roaring::Roaring({1, 3, 5})}});
   const uint32_t row_count = 7;

   OperatorVector input = generateTestInput(test_bitmaps, row_count);
   const Union under_test(std::move(input), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 3, 5}));
}

TEST(OperatorUnion, evaluateShouldReturnCorrectValues1) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 3}), roaring::Roaring({1, 2, 3})}}
   );
   const uint32_t row_count = 7;

   OperatorVector input = generateTestInput(test_bitmaps, row_count);
   const Union under_test(std::move(input), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 2, 3}));
}

TEST(OperatorUnion, evaluateShouldReturnCorrectValues2) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 7}), roaring::Roaring({1, 3}), roaring::Roaring({3})}}
   );
   const uint32_t row_count = 8;

   OperatorVector input = generateTestInput(test_bitmaps, row_count);
   const Union under_test(std::move(input), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 3, 7}));
}

TEST(OperatorUnion, evaluateShouldReturnCorrectValuesMany) {
   const std::vector<roaring::Roaring> test_bitmaps({{
      roaring::Roaring(),
      roaring::Roaring({3}),
      roaring::Roaring({4}),
      roaring::Roaring({2, 4}),
      roaring::Roaring({2, 4}),
      roaring::Roaring({2, 4}),
      roaring::Roaring({2, 4}),
      roaring::Roaring({2, 4}),
      roaring::Roaring({2, 4}),
      roaring::Roaring({2, 4}),
      roaring::Roaring({2, 4}),
      roaring::Roaring({2, 4}),
      roaring::Roaring({2, 4}),
      roaring::Roaring({2, 4}),
   }});
   const uint32_t row_count = 13;

   OperatorVector input = generateTestInput(test_bitmaps, row_count);
   const Union under_test(std::move(input), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({2, 3, 4}));
}

TEST(OperatorUnion, evaluateShouldReturnCorrectValuesEmptyInput) {
   const std::vector<roaring::Roaring> test_bitmaps({{roaring::Roaring()}});
   const uint32_t row_count = 80;

   OperatorVector input = generateTestInput(test_bitmaps, row_count);
   const Union under_test(std::move(input), row_count);
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring());
}

TEST(OperatorUnion, correctTypeInfo) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 2, 3})}}
   );
   const uint32_t row_count = 5;

   OperatorVector input = generateTestInput(test_bitmaps, row_count);
   const Union under_test(std::move(input), row_count);

   ASSERT_EQ(under_test.type(), silo::query_engine::filter::operators::UNION);
}
