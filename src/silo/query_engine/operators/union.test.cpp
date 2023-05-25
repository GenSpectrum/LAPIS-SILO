#include "silo/query_engine/operators/union.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

#include "silo/query_engine/operators/index_scan.h"
#include "silo/query_engine/query_compilation_exception.h"

using silo::query_engine::operators::IndexScan;
using silo::query_engine::operators::Operator;
using silo::query_engine::operators::Union;

using OperatorVector = std::vector<std::unique_ptr<Operator>>;

namespace {
OperatorVector generateTestInput(const std::vector<roaring::Roaring>& bitmaps) {
   OperatorVector result;
   std::transform(
      bitmaps.begin(),
      bitmaps.end(),
      std::back_inserter(result),
      [&](const auto& bitmap) { return std::make_unique<IndexScan>(&bitmap); }
   );
   return result;
}
}  // namespace

TEST(OperatorUnion, evaluatesCorrectOnEmptyInput) {
   OperatorVector input;

   const Union under_test(std::move(input));
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring());
}

TEST(OperatorUnion, evaluatesCorrectOnOneInput) {
   const std::vector<roaring::Roaring> test_bitmaps({{roaring::Roaring({1, 3, 5})}});

   OperatorVector input = generateTestInput(test_bitmaps);
   const Union under_test(std::move(input));
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 3, 5}));
}

TEST(OperatorUnion, evaluateShouldReturnCorrectValues1) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 3}), roaring::Roaring({1, 2, 3})}}
   );

   OperatorVector input = generateTestInput(test_bitmaps);
   const Union under_test(std::move(input));
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 2, 3}));
}

TEST(OperatorUnion, evaluateShouldReturnCorrectValues2) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 7}), roaring::Roaring({1, 3}), roaring::Roaring({3})}}
   );

   OperatorVector input = generateTestInput(test_bitmaps);
   const Union under_test(std::move(input));
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

   OperatorVector input = generateTestInput(test_bitmaps);
   const Union under_test(std::move(input));
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({2, 3, 4}));
}

TEST(OperatorUnion, evaluateShouldReturnCorrectValuesEmptyInput) {
   const std::vector<roaring::Roaring> test_bitmaps({{roaring::Roaring()}});

   OperatorVector input = generateTestInput(test_bitmaps);
   const Union under_test(std::move(input));
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring());
}

TEST(OperatorUnion, correctTypeInfo) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 2, 3})}}
   );

   OperatorVector input = generateTestInput(test_bitmaps);
   const Union under_test(std::move(input));

   ASSERT_EQ(under_test.type(), silo::query_engine::operators::UNION);
}
