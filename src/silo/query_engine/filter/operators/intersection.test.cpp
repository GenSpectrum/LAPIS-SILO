#include "silo/query_engine/filter/operators/intersection.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/query_compilation_exception.h"

using silo::query_engine::CopyOnWriteBitmap;
using silo::query_engine::QueryCompilationException;
using silo::query_engine::filter::operators::IndexScan;
using silo::query_engine::filter::operators::Intersection;
using silo::query_engine::filter::operators::OperatorVector;

namespace {
OperatorVector generateTestInput(const std::vector<roaring::Roaring>& bitmaps, uint32_t row_count) {
   OperatorVector result;

   std::ranges::transform(bitmaps, std::back_inserter(result), [&](const auto& bitmap) {
      return std::make_unique<IndexScan>(CopyOnWriteBitmap{&bitmap}, row_count);
   });
   return result;
}
}  // namespace

TEST(OperatorIntersection, shouldFailOnEmptyInput) {
   OperatorVector non_negated;
   OperatorVector negated;
   const uint32_t row_count = 5;

   ASSERT_THROW(
      const Intersection under_test(std::move(non_negated), std::move(negated), row_count),
      QueryCompilationException
   );
}

TEST(OperatorIntersection, shouldFailOnOnlyNegated) {
   const std::vector<roaring::Roaring> test_negated_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 2, 3})}}
   );
   const uint32_t row_count = 5;

   OperatorVector non_negated;
   OperatorVector negated = generateTestInput(test_negated_bitmaps, row_count);
   ASSERT_THROW(
      const Intersection under_test(std::move(non_negated), std::move(negated), row_count),
      QueryCompilationException
   );
}

TEST(OperatorIntersection, shouldFailOnOneNonNegated) {
   const std::vector<roaring::Roaring> test_bitmaps({{roaring::Roaring({1, 2, 3})}});
   const uint32_t row_count = 5;

   OperatorVector non_negated = generateTestInput(test_bitmaps, row_count);
   OperatorVector negated;
   ASSERT_THROW(
      const Intersection under_test(std::move(non_negated), std::move(negated), row_count),
      std::runtime_error
   );
}

TEST(OperatorIntersection, evaluateShouldReturnCorrectValuesNoNegated) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 3}), roaring::Roaring({1, 2, 3})}}
   );
   const uint32_t row_count = 5;

   OperatorVector non_negated = generateTestInput(test_bitmaps, row_count);
   OperatorVector negated;
   const Intersection under_test(std::move(non_negated), std::move(negated), row_count);
   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring({1, 3}));
}

TEST(OperatorIntersection, evaluateShouldReturnCorrectValues) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 3}), roaring::Roaring({1, 2, 3})}}
   );
   const std::vector<roaring::Roaring> test_negated_bitmaps(
      {{roaring::Roaring(), roaring::Roaring({3})}}
   );
   const uint32_t row_count = 5;

   OperatorVector non_negated = generateTestInput(test_bitmaps, row_count);
   OperatorVector negated = generateTestInput(test_negated_bitmaps, row_count);
   const Intersection under_test(std::move(non_negated), std::move(negated), row_count);
   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring({1}));
}

TEST(OperatorIntersection, evaluateShouldReturnCorrectValuesManyNegated) {
   const std::vector<roaring::Roaring> test_bitmaps({{roaring::Roaring({1, 2, 3})}});
   const std::vector<roaring::Roaring> test_negated_bitmaps({{
      roaring::Roaring(),
      roaring::Roaring({3}),
      roaring::Roaring({4}),
      roaring::Roaring({2, 4}),
   }});
   const uint32_t row_count = 5;

   OperatorVector non_negated = generateTestInput(test_bitmaps, row_count);
   OperatorVector negated = generateTestInput(test_negated_bitmaps, row_count);
   const Intersection under_test(std::move(non_negated), std::move(negated), row_count);
   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring({1}));
}

TEST(OperatorIntersection, evaluateShouldReturnCorrectValuesEmptyInput) {
   const std::vector<roaring::Roaring> test_bitmaps({{roaring::Roaring()}});
   const std::vector<roaring::Roaring> test_negated_bitmaps({{
      roaring::Roaring({3}),
      roaring::Roaring({4}),
      roaring::Roaring({2, 4}),
   }});
   const uint32_t row_count = 5;

   OperatorVector non_negated = generateTestInput(test_bitmaps, row_count);
   OperatorVector negated = generateTestInput(test_negated_bitmaps, row_count);
   const Intersection under_test(std::move(non_negated), std::move(negated), row_count);
   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring());
}

TEST(OperatorIntersection, correctTypeInfo) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 2, 3})}}
   );
   const uint32_t row_count = 5;

   OperatorVector non_negated = generateTestInput(test_bitmaps, row_count);
   OperatorVector negated;
   const Intersection under_test(std::move(non_negated), std::move(negated), row_count);

   ASSERT_EQ(under_test.type(), silo::query_engine::filter::operators::INTERSECTION);
}
