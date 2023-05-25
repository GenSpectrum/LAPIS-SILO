#include "silo/query_engine/operators/bitmap_selection.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::operators::BitmapSelection;

TEST(OperatorBitmapSelection, containsCheckShouldReturnCorrectValues) {
   std::vector<roaring::Roaring> test_bitmaps({{
      roaring::Roaring({1, 2, 3}),
      roaring::Roaring({1, 3}),
      roaring::Roaring({1, 2, 3}),
      roaring::Roaring({}),
      roaring::Roaring({3}),
      roaring::Roaring({4}),
      roaring::Roaring({1, 4}),
      roaring::Roaring({2, 4}),
   }});

   BitmapSelection under_test(
      test_bitmaps.data(), test_bitmaps.size(), BitmapSelection::CONTAINS, 2
   );
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 2, 7}));
   under_test.negate();
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 3, 4, 5, 6}));
}

TEST(OperatorBitmapSelection, notContainsCheckShouldReturnCorrectValues) {
   std::vector<roaring::Roaring> test_bitmaps({{
      roaring::Roaring({1, 2, 3}),
      roaring::Roaring({1, 3}),
      roaring::Roaring({1, 2, 3}),
      roaring::Roaring({}),
      roaring::Roaring({3}),
      roaring::Roaring({4}),
      roaring::Roaring({1, 4}),
      roaring::Roaring({2, 4}),
   }});

   BitmapSelection under_test(
      test_bitmaps.data(), test_bitmaps.size(), BitmapSelection::NOT_CONTAINS, 2
   );
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({1, 3, 4, 5, 6}));
   under_test.negate();
   ASSERT_EQ(*under_test.evaluate(), roaring::Roaring({0, 2, 7}));
}

TEST(OperatorBitmapSelection, correctTypeInfo) {
   std::vector<roaring::Roaring> test_bitmaps({{
      roaring::Roaring({1, 2, 3}),
      roaring::Roaring({1, 3}),
      roaring::Roaring({1, 2, 3}),
      roaring::Roaring({}),
      roaring::Roaring({3}),
      roaring::Roaring({4}),
      roaring::Roaring({1, 4}),
      roaring::Roaring({2, 4}),
   }});

   BitmapSelection under_test(
      test_bitmaps.data(), test_bitmaps.size(), BitmapSelection::NOT_CONTAINS, 2
   );

   ASSERT_EQ(under_test.type(), silo::query_engine::operators::BITMAP_SELECTION);
   under_test.negate();
   ASSERT_EQ(under_test.type(), silo::query_engine::operators::BITMAP_SELECTION);
}
