#include "silo/query_engine/operators/intersection.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

#include "silo/query_engine/operators/index_scan.h"
#include "silo/query_engine/query_compilation_exception.h"

using silo::query_engine::operators::IndexScan;
using silo::query_engine::operators::Intersection;
using silo::query_engine::operators::Operator;

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

TEST(OperatorIntersection, shouldFailOnEmptyInput) {
   OperatorVector non_negated;
   OperatorVector negated;
   ASSERT_THROW(
      const Intersection under_test(std::move(non_negated), std::move(negated)),
      silo::QueryCompilationException
   );
}

TEST(OperatorIntersection, shouldFailOnOnlyNegated) {
   const std::vector<roaring::Roaring> test_negated_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 2, 3})}}
   );

   OperatorVector non_negated;
   OperatorVector negated = generateTestInput(test_negated_bitmaps);
   ASSERT_THROW(
      const Intersection under_test(std::move(non_negated), std::move(negated)),
      silo::QueryCompilationException
   );
}

TEST(OperatorIntersection, shouldFailOnOneNonNegated) {
   const std::vector<roaring::Roaring> test_bitmaps({{roaring::Roaring({1, 2, 3})}});

   OperatorVector non_negated = generateTestInput(test_bitmaps);
   OperatorVector negated;
   ASSERT_THROW(
      const Intersection under_test(std::move(non_negated), std::move(negated)), std::runtime_error
   );
}

TEST(OperatorIntersection, evaluateShouldReturnCorrectValuesNoNegated) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 3}), roaring::Roaring({1, 2, 3})}}
   );

   OperatorVector non_negated = generateTestInput(test_bitmaps);
   OperatorVector negated;
   const Intersection under_test(std::move(non_negated), std::move(negated));
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 3}));
}

TEST(OperatorIntersection, evaluateShouldReturnCorrectValues) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 3}), roaring::Roaring({1, 2, 3})}}
   );
   const std::vector<roaring::Roaring> test_negated_bitmaps(
      {{roaring::Roaring(), roaring::Roaring({3})}}
   );

   OperatorVector non_negated = generateTestInput(test_bitmaps);
   OperatorVector negated = generateTestInput(test_negated_bitmaps);
   const Intersection under_test(std::move(non_negated), std::move(negated));
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1}));
}

TEST(OperatorIntersection, evaluateShouldReturnCorrectValuesManyNegated) {
   const std::vector<roaring::Roaring> test_bitmaps({{roaring::Roaring({1, 2, 3})}});
   const std::vector<roaring::Roaring> test_negated_bitmaps({{
      roaring::Roaring(),
      roaring::Roaring({3}),
      roaring::Roaring({4}),
      roaring::Roaring({2, 4}),
   }});

   OperatorVector non_negated = generateTestInput(test_bitmaps);
   OperatorVector negated = generateTestInput(test_negated_bitmaps);
   const Intersection under_test(std::move(non_negated), std::move(negated));
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1}));
}

TEST(OperatorIntersection, evaluateShouldReturnCorrectValuesEmptyInput) {
   const std::vector<roaring::Roaring> test_bitmaps({{roaring::Roaring()}});
   const std::vector<roaring::Roaring> test_negated_bitmaps({{
      roaring::Roaring({3}),
      roaring::Roaring({4}),
      roaring::Roaring({2, 4}),
   }});

   OperatorVector non_negated = generateTestInput(test_bitmaps);
   OperatorVector negated = generateTestInput(test_negated_bitmaps);
   const Intersection under_test(std::move(non_negated), std::move(negated));
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring());
}

TEST(OperatorIntersection, correctTypeInfo) {
   const std::vector<roaring::Roaring> test_bitmaps(
      {{roaring::Roaring({1, 2, 3}), roaring::Roaring({1, 2, 3})}}
   );

   OperatorVector non_negated = generateTestInput(test_bitmaps);
   OperatorVector negated;
   const Intersection under_test(std::move(non_negated), std::move(negated));

   ASSERT_EQ(under_test.type(), silo::query_engine::operators::INTERSECTION);
}
