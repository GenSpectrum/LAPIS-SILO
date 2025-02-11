#include "silo/query_engine/filter/operators/bitmap_selection.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::filter::operators::BitmapSelection;

TEST(OperatorBitmapSelection, containsCheckShouldReturnCorrectValues) {
   const std::vector<roaring::Roaring> test_bitmaps({{
      roaring::Roaring({1, 2, 3}),
      roaring::Roaring({1, 3}),
      roaring::Roaring({1, 2, 3}),
      roaring::Roaring({}),
      roaring::Roaring({3}),
      roaring::Roaring({4}),
      roaring::Roaring({1, 4}),
      roaring::Roaring({2, 4}),
   }});

   auto under_test = std::make_unique<BitmapSelection>(
      test_bitmaps.data(), test_bitmaps.size(), BitmapSelection::CONTAINS, 2
   );
   ASSERT_EQ(*under_test->evaluate(), roaring::Roaring({0, 2, 7}));
   auto negated = BitmapSelection::negate(std::move(under_test));
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({1, 3, 4, 5, 6}));
}

TEST(OperatorBitmapSelection, notContainsCheckShouldReturnCorrectValues) {
   const std::vector<roaring::Roaring> test_bitmaps({{
      roaring::Roaring({1, 2, 3}),
      roaring::Roaring({1, 3}),
      roaring::Roaring({1, 2, 3}),
      roaring::Roaring({}),
      roaring::Roaring({3}),
      roaring::Roaring({4}),
      roaring::Roaring({1, 4}),
      roaring::Roaring({2, 4}),
   }});

   auto under_test = std::make_unique<BitmapSelection>(
      test_bitmaps.data(), test_bitmaps.size(), BitmapSelection::NOT_CONTAINS, 2
   );
   ASSERT_EQ(*under_test->evaluate(), roaring::Roaring({1, 3, 4, 5, 6}));
   auto negated = BitmapSelection::negate(std::move(under_test));
   ASSERT_EQ(*negated->evaluate(), roaring::Roaring({0, 2, 7}));
}

TEST(OperatorBitmapSelection, correctTypeInfo) {
   const std::vector<roaring::Roaring> test_bitmaps({{
      roaring::Roaring({1, 2, 3}),
      roaring::Roaring({1, 3}),
      roaring::Roaring({1, 2, 3}),
      roaring::Roaring({}),
      roaring::Roaring({3}),
      roaring::Roaring({4}),
      roaring::Roaring({1, 4}),
      roaring::Roaring({2, 4}),
   }});

   auto under_test = std::make_unique<BitmapSelection>(
      test_bitmaps.data(), test_bitmaps.size(), BitmapSelection::NOT_CONTAINS, 2
   );

   ASSERT_EQ(under_test->type(), silo::query_engine::filter::operators::BITMAP_SELECTION);
   auto negated = BitmapSelection::negate(std::move(under_test));
   ASSERT_EQ(negated->type(), silo::query_engine::filter::operators::BITMAP_SELECTION);
}
