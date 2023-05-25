#include "silo/query_engine/operators/threshold.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

#include "silo/query_engine/operators/index_scan.h"
#include "silo/query_engine/query_compilation_exception.h"

using silo::query_engine::operators::IndexScan;
using silo::query_engine::operators::Operator;
using silo::query_engine::operators::Threshold;

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

TEST(OperatorThreshold, evaluatesCorrectOnEmptyInput) {
   OperatorVector non_negated;
   OperatorVector negated;

   ASSERT_THROW(
      const Threshold under_test(std::move(non_negated), std::move(negated), 2, true, 0),
      silo::QueryCompilationException
   );
}

TEST(OperatorThreshold, evaluatesCorrectOnlyNegated) {
   const std::vector<roaring::Roaring> test_negated_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 3})}}
   );

   const Threshold under_test_1_exact(
      OperatorVector(), generateTestInput(test_negated_bitmaps), 1, true, 4
   );
   ASSERT_EQ(*under_test_1_exact.evaluate(), roaring::Roaring({2}));

   const Threshold under_test_1_or_more(
      OperatorVector(), generateTestInput(test_negated_bitmaps), 1, false, 4
   );
   ASSERT_EQ(*under_test_1_or_more.evaluate(), roaring::Roaring({0, 2}));
}

/*
TEST(OperatorThreshold, correctOnOneNonNegated) {
   const std::vector<roaring::Roaring> test_bitmaps({{roaring::Roaring({1, 2, 3})}});

   const Threshold under_test_1_exact(
      generateTestInput(test_bitmaps), OperatorVector(), 1, true, 4
   );
   ASSERT_EQ(*under_test_1_exact.evaluate(), roaring::Roaring({1, 2, 3}));

   const Threshold under_test_0_or_more(
      generateTestInput(test_bitmaps), OperatorVector(), 0, false, 4
   );
   ASSERT_EQ(*under_test_0_or_more.evaluate(), roaring::Roaring({0, 1, 2, 3}));

   const Threshold under_test_1_or_more(
      generateTestInput(test_bitmaps), OperatorVector(), 1, false, 4
   );
   ASSERT_EQ(*under_test_1_or_more.evaluate(), roaring::Roaring({1, 2, 3}));
}*/

TEST(OperatorThreshold, evaluateShouldReturnCorrectValuesNoNegated) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2}), roaring::Roaring({1, 3}), roaring::Roaring({1, 2, 3})}}
   );

   const Threshold under_test_1_exact(
      generateTestInput(test_bitmaps), OperatorVector(), 1, true, 4
   );
   ASSERT_EQ(*under_test_1_exact.evaluate(), roaring::Roaring({}));

   const Threshold under_test_2_exact(
      generateTestInput(test_bitmaps), OperatorVector(), 2, true, 4
   );
   ASSERT_EQ(*under_test_2_exact.evaluate(), roaring::Roaring({2, 3}));

   const Threshold under_test_1_or_more(
      generateTestInput(test_bitmaps), OperatorVector(), 1, false, 4
   );
   ASSERT_EQ(*under_test_1_or_more.evaluate(), roaring::Roaring({1, 2, 3}));

   const Threshold under_test_2_or_more(
      generateTestInput(test_bitmaps), OperatorVector(), 2, false, 4
   );
   ASSERT_EQ(*under_test_2_or_more.evaluate(), roaring::Roaring({1, 2, 3}));
}

TEST(OperatorThreshold, evaluateShouldReturnCorrectValues) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 3}), roaring::Roaring({1, 2, 3})}}
   );
   const std::vector<roaring::Roaring> test_negated_bitmaps(
      {{roaring::Roaring(), roaring::Roaring({3})}}
   );

   const Threshold under_test_1_exact(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 1, true, 4
   );
   ASSERT_EQ(*under_test_1_exact.evaluate(), roaring::Roaring({}));

   const Threshold under_test_2_exact(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 2, true, 4
   );
   ASSERT_EQ(*under_test_2_exact.evaluate(), roaring::Roaring({0}));

   const Threshold under_test_3_exact(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 3, true, 4
   );
   ASSERT_EQ(*under_test_3_exact.evaluate(), roaring::Roaring({}));

   const Threshold under_test_4_exact(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 4, true, 4
   );
   ASSERT_EQ(*under_test_4_exact.evaluate(), roaring::Roaring({2, 3}));

   const Threshold under_test_1_or_more(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 1, false, 4
   );
   ASSERT_EQ(*under_test_1_or_more.evaluate(), roaring::Roaring({0, 1, 2, 3}));

   const Threshold under_test_2_or_more(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 2, false, 4
   );
   ASSERT_EQ(*under_test_2_or_more.evaluate(), roaring::Roaring({0, 1, 2, 3}));

   const Threshold under_test_3_or_more(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 3, false, 4
   );
   ASSERT_EQ(*under_test_3_or_more.evaluate(), roaring::Roaring({1, 2, 3}));

   const Threshold under_test_4_or_more(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 4, false, 4
   );
   ASSERT_EQ(*under_test_4_or_more.evaluate(), roaring::Roaring({1, 2, 3}));
}

TEST(OperatorThreshold, evaluateShouldReturnCorrectValuesManyNegated) {
   const std::vector<roaring::Roaring> test_bitmaps({{roaring::Roaring({1, 2, 3})}});
   const std::vector<roaring::Roaring> test_negated_bitmaps({{
      roaring::Roaring(),
      roaring::Roaring({3}),
      roaring::Roaring({4}),
      roaring::Roaring({2, 4}),
   }});

   const Threshold under_test_1_exact(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 1, true, 5
   );
   ASSERT_EQ(*under_test_1_exact.evaluate(), roaring::Roaring({}));

   const Threshold under_test_2_exact(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 2, true, 5
   );
   ASSERT_EQ(*under_test_2_exact.evaluate(), roaring::Roaring({4}));

   const Threshold under_test_3_exact(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 3, true, 5
   );
   ASSERT_EQ(*under_test_3_exact.evaluate(), roaring::Roaring({}));

   const Threshold under_test_4_exact(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 4, true, 5
   );
   ASSERT_EQ(*under_test_4_exact.evaluate(), roaring::Roaring({0, 2, 3}));

   const Threshold under_test_1_or_more(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 1, false, 5
   );
   ASSERT_EQ(*under_test_1_or_more.evaluate(), roaring::Roaring({0, 1, 2, 3, 4}));

   const Threshold under_test_2_or_more(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 2, false, 5
   );
   ASSERT_EQ(*under_test_2_or_more.evaluate(), roaring::Roaring({0, 1, 2, 3, 4}));

   const Threshold under_test_3_or_more(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 3, false, 5
   );
   ASSERT_EQ(*under_test_3_or_more.evaluate(), roaring::Roaring({0, 1, 2, 3}));

   const Threshold under_test_4_or_more(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 4, false, 5
   );
   ASSERT_EQ(*under_test_4_or_more.evaluate(), roaring::Roaring({0, 1, 2, 3}));
}

TEST(OperatorThreshold, evaluateShouldReturnCorrectValuesEmptyInput) {
   const std::vector<roaring::Roaring> test_bitmaps({{roaring::Roaring()}});
   const std::vector<roaring::Roaring> test_negated_bitmaps({{
      roaring::Roaring({3}),
      roaring::Roaring({4}),
      roaring::Roaring({2, 4}),
   }});

   const Threshold under_test_1_exact(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 1, true, 4
   );
   ASSERT_EQ(*under_test_1_exact.evaluate(), roaring::Roaring({4}));

   const Threshold under_test_2_exact(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 2, true, 4
   );
   ASSERT_EQ(*under_test_2_exact.evaluate(), roaring::Roaring({2, 3}));

   const Threshold under_test_3_exact(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 3, true, 4
   );
   ASSERT_EQ(*under_test_3_exact.evaluate(), roaring::Roaring({0, 1}));

   const Threshold under_test_1_or_more(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 1, false, 4
   );
   ASSERT_EQ(*under_test_1_or_more.evaluate(), roaring::Roaring({0, 1, 2, 3, 4}));

   const Threshold under_test_2_or_more(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 2, false, 4
   );
   ASSERT_EQ(*under_test_2_or_more.evaluate(), roaring::Roaring({0, 1, 2, 3}));

   const Threshold under_test_3_or_more(
      generateTestInput(test_bitmaps), generateTestInput(test_negated_bitmaps), 3, false, 4
   );
   ASSERT_EQ(*under_test_3_or_more.evaluate(), roaring::Roaring({0, 1}));
}

TEST(OperatorThreshold, correctTypeInfo) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 2, 3})}}
   );

   const Threshold under_test(generateTestInput(test_bitmaps), OperatorVector(), 1, true, 4);

   ASSERT_EQ(under_test.type(), silo::query_engine::operators::THRESHOLD);
}
