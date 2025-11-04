#include "silo/query_engine/filter/operators/threshold.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/query_compilation_exception.h"

using silo::query_engine::CopyOnWriteBitmap;
using silo::query_engine::filter::operators::IndexScan;
using silo::query_engine::filter::operators::Operator;
using silo::query_engine::filter::operators::Threshold;

using silo::query_engine::filter::operators::OperatorVector;

namespace {
OperatorVector generateTestInput(
   const std::vector<roaring::Roaring>& bitmaps,
   const uint32_t row_count
) {
   OperatorVector result;
   std::ranges::transform(bitmaps, std::back_inserter(result), [&](const auto& bitmap) {
      return std::make_unique<IndexScan>(CopyOnWriteBitmap{&bitmap}, row_count);
   });
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
   const uint32_t row_count = 4;

   const Threshold under_test_1_exact(
      OperatorVector(), generateTestInput(test_negated_bitmaps, row_count), 1, true, row_count
   );
   ASSERT_EQ(under_test_1_exact.evaluate().getConstReference(), roaring::Roaring({2}));

   const Threshold under_test_1_or_more(
      OperatorVector(), generateTestInput(test_negated_bitmaps, row_count), 1, false, row_count
   );
   ASSERT_EQ(under_test_1_or_more.evaluate().getConstReference(), roaring::Roaring({0, 2}));
}

TEST(OperatorThreshold, evaluateShouldReturnCorrectValuesNoNegated) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2}), roaring::Roaring({1, 3}), roaring::Roaring({1, 2, 3})}}
   );
   const uint32_t row_count = 4;

   const Threshold under_test_1_exact(
      generateTestInput(test_bitmaps, row_count), OperatorVector(), 1, true, row_count
   );
   ASSERT_EQ(under_test_1_exact.evaluate().getConstReference(), roaring::Roaring({}));

   const Threshold under_test_2_exact(
      generateTestInput(test_bitmaps, row_count), OperatorVector(), 2, true, row_count
   );
   ASSERT_EQ(under_test_2_exact.evaluate().getConstReference(), roaring::Roaring({2, 3}));

   const Threshold under_test_1_or_more(
      generateTestInput(test_bitmaps, row_count), OperatorVector(), 1, false, row_count
   );
   ASSERT_EQ(under_test_1_or_more.evaluate().getConstReference(), roaring::Roaring({1, 2, 3}));

   const Threshold under_test_2_or_more(
      generateTestInput(test_bitmaps, row_count), OperatorVector(), 2, false, row_count
   );
   ASSERT_EQ(under_test_2_or_more.evaluate().getConstReference(), roaring::Roaring({1, 2, 3}));
}

TEST(OperatorThreshold, evaluateShouldReturnCorrectValues) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 3}), roaring::Roaring({1, 2, 3})}}
   );
   const std::vector<roaring::Roaring> test_negated_bitmaps(
      {{roaring::Roaring(), roaring::Roaring({3})}}
   );
   const uint32_t row_count = 4;

   const Threshold under_test_1_exact(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      1,
      true,
      row_count
   );
   ASSERT_EQ(under_test_1_exact.evaluate().getConstReference(), roaring::Roaring({}));

   const Threshold under_test_2_exact(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      2,
      true,
      row_count
   );
   ASSERT_EQ(under_test_2_exact.evaluate().getConstReference(), roaring::Roaring({0}));

   const Threshold under_test_3_exact(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      3,
      true,
      row_count
   );
   ASSERT_EQ(under_test_3_exact.evaluate().getConstReference(), roaring::Roaring({}));

   const Threshold under_test_4_exact(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      4,
      true,
      row_count
   );
   ASSERT_EQ(under_test_4_exact.evaluate().getConstReference(), roaring::Roaring({2, 3}));

   const Threshold under_test_1_or_more(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      1,
      false,
      row_count
   );
   ASSERT_EQ(under_test_1_or_more.evaluate().getConstReference(), roaring::Roaring({0, 1, 2, 3}));

   const Threshold under_test_2_or_more(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      2,
      false,
      row_count
   );
   ASSERT_EQ(under_test_2_or_more.evaluate().getConstReference(), roaring::Roaring({0, 1, 2, 3}));

   const Threshold under_test_3_or_more(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      3,
      false,
      row_count
   );
   ASSERT_EQ(under_test_3_or_more.evaluate().getConstReference(), roaring::Roaring({1, 2, 3}));

   const Threshold under_test_4_or_more(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      4,
      false,
      row_count
   );
   ASSERT_EQ(under_test_4_or_more.evaluate().getConstReference(), roaring::Roaring({1, 2, 3}));
}

TEST(OperatorThreshold, evaluateShouldReturnCorrectValuesManyNegated) {
   const std::vector<roaring::Roaring> test_bitmaps({{roaring::Roaring({1, 2, 3})}});
   const std::vector<roaring::Roaring> test_negated_bitmaps({{
      roaring::Roaring(),
      roaring::Roaring({3}),
      roaring::Roaring({4}),
      roaring::Roaring({2, 4}),
   }});
   const uint32_t row_count = 5;

   const Threshold under_test_1_exact(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      1,
      true,
      row_count
   );
   ASSERT_EQ(under_test_1_exact.evaluate().getConstReference(), roaring::Roaring({}));

   const Threshold under_test_2_exact(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      2,
      true,
      row_count
   );
   ASSERT_EQ(under_test_2_exact.evaluate().getConstReference(), roaring::Roaring({4}));

   const Threshold under_test_3_exact(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      3,
      true,
      row_count
   );
   ASSERT_EQ(under_test_3_exact.evaluate().getConstReference(), roaring::Roaring({}));

   const Threshold under_test_4_exact(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      4,
      true,
      row_count
   );
   ASSERT_EQ(under_test_4_exact.evaluate().getConstReference(), roaring::Roaring({0, 2, 3}));

   const Threshold under_test_1_or_more(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      1,
      false,
      row_count
   );
   ASSERT_EQ(
      under_test_1_or_more.evaluate().getConstReference(), roaring::Roaring({0, 1, 2, 3, 4})
   );

   const Threshold under_test_2_or_more(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      2,
      false,
      row_count
   );
   ASSERT_EQ(
      under_test_2_or_more.evaluate().getConstReference(), roaring::Roaring({0, 1, 2, 3, 4})
   );

   const Threshold under_test_3_or_more(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      3,
      false,
      row_count
   );
   ASSERT_EQ(under_test_3_or_more.evaluate().getConstReference(), roaring::Roaring({0, 1, 2, 3}));

   const Threshold under_test_4_or_more(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      4,
      false,
      row_count
   );
   ASSERT_EQ(under_test_4_or_more.evaluate().getConstReference(), roaring::Roaring({0, 1, 2, 3}));
}

TEST(OperatorThreshold, evaluateShouldReturnCorrectValuesEmptyInput) {
   const std::vector<roaring::Roaring> test_bitmaps({{roaring::Roaring()}});
   const std::vector<roaring::Roaring> test_negated_bitmaps({{
      roaring::Roaring({3}),
      roaring::Roaring({4}),
      roaring::Roaring({2, 4}),
   }});
   const uint32_t row_count = 4;

   const Threshold under_test_1_exact(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      1,
      true,
      row_count
   );
   ASSERT_EQ(under_test_1_exact.evaluate().getConstReference(), roaring::Roaring({4}));

   const Threshold under_test_2_exact(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      2,
      true,
      row_count
   );
   ASSERT_EQ(under_test_2_exact.evaluate().getConstReference(), roaring::Roaring({2, 3}));

   const Threshold under_test_3_exact(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      3,
      true,
      row_count
   );
   ASSERT_EQ(under_test_3_exact.evaluate().getConstReference(), roaring::Roaring({0, 1}));

   const Threshold under_test_1_or_more(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      1,
      false,
      row_count
   );
   ASSERT_EQ(
      under_test_1_or_more.evaluate().getConstReference(), roaring::Roaring({0, 1, 2, 3, 4})
   );

   const Threshold under_test_2_or_more(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      2,
      false,
      row_count
   );
   ASSERT_EQ(under_test_2_or_more.evaluate().getConstReference(), roaring::Roaring({0, 1, 2, 3}));

   const Threshold under_test_3_or_more(
      generateTestInput(test_bitmaps, row_count),
      generateTestInput(test_negated_bitmaps, row_count),
      3,
      false,
      row_count
   );
   ASSERT_EQ(under_test_3_or_more.evaluate().getConstReference(), roaring::Roaring({0, 1}));
}

TEST(OperatorThreshold, correctTypeInfo) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 2, 3})}}
   );
   const uint32_t row_count = 4;

   const Threshold under_test(
      generateTestInput(test_bitmaps, row_count), OperatorVector(), 1, true, row_count
   );

   ASSERT_EQ(under_test.type(), silo::query_engine::filter::operators::THRESHOLD);
}
