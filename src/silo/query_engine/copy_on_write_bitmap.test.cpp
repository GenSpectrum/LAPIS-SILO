#include "silo/query_engine/copy_on_write_bitmap.h"

#include <cstdint>
#include <vector>

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::query_engine::CopyOnWriteBitmap;

namespace {

// Values spanning several 2^16 container blocks, so the operations are exercised across containers
// present in only one operand and in both.
roaring::Roaring multiContainer() {
   roaring::Roaring bitmap{1, 5, 100};
   bitmap.add((1U << 16) + 3);
   bitmap.add((3U << 16) + 7);
   return bitmap;
}

}  // namespace

TEST(CopyOnWriteBitmap, cardinalityAndEmptiness) {
   const roaring::Roaring source = multiContainer();
   const CopyOnWriteBitmap under_test{&source};
   EXPECT_EQ(under_test.cardinality(), source.cardinality());
   EXPECT_FALSE(under_test.isEmpty());

   const CopyOnWriteBitmap empty;
   EXPECT_TRUE(empty.isEmpty());
   EXPECT_EQ(empty.cardinality(), 0);
}

TEST(CopyOnWriteBitmap, toRoaringMaterializesEqualCopy) {
   const roaring::Roaring source = multiContainer();
   const CopyOnWriteBitmap under_test{&source};
   EXPECT_EQ(under_test.toRoaring(), source);
}

TEST(CopyOnWriteBitmap, intersection) {
   const roaring::Roaring left = multiContainer();
   const roaring::Roaring right{5, 100, (1U << 16) + 3, 999};

   CopyOnWriteBitmap in_place{&left};
   in_place &= CopyOnWriteBitmap{&right};
   EXPECT_EQ(in_place.toRoaring(), left & right);

   const CopyOnWriteBitmap fresh = CopyOnWriteBitmap{&left} & CopyOnWriteBitmap{&right};
   EXPECT_EQ(fresh.toRoaring(), left & right);
   EXPECT_EQ(
      CopyOnWriteBitmap{&left}.andCardinality(CopyOnWriteBitmap{&right}),
      (left & right).cardinality()
   );
}

TEST(CopyOnWriteBitmap, difference) {
   const roaring::Roaring left = multiContainer();
   const roaring::Roaring right{5, (1U << 16) + 3};

   CopyOnWriteBitmap in_place{&left};
   in_place -= CopyOnWriteBitmap{&right};
   EXPECT_EQ(in_place.toRoaring(), left - right);

   const CopyOnWriteBitmap fresh = CopyOnWriteBitmap{&left} - CopyOnWriteBitmap{&right};
   EXPECT_EQ(fresh.toRoaring(), left - right);
}

TEST(CopyOnWriteBitmap, unionInPlaceAndFastUnion) {
   const roaring::Roaring first{1, 5};
   const roaring::Roaring second = multiContainer();
   const roaring::Roaring third{5, (3U << 16) + 7, (4U << 16) + 1};

   CopyOnWriteBitmap in_place{&first};
   in_place |= CopyOnWriteBitmap{&second};
   EXPECT_EQ(in_place.toRoaring(), first | second);

   std::vector<CopyOnWriteBitmap> bitmaps;
   bitmaps.emplace_back(&first);
   bitmaps.emplace_back(&second);
   bitmaps.emplace_back(&third);
   EXPECT_EQ(CopyOnWriteBitmap::fastUnion(bitmaps).toRoaring(), first | second | third);
}

TEST(CopyOnWriteBitmap, iteratesRowIdsInAscendingOrder) {
   const roaring::Roaring source = multiContainer();
   const CopyOnWriteBitmap under_test{&source};

   std::vector<uint32_t> iterated;
   for (const uint32_t row : under_test) {
      iterated.push_back(row);
   }
   EXPECT_EQ(iterated, (std::vector<uint32_t>{1, 5, 100, (1U << 16) + 3, (3U << 16) + 7}));

   const CopyOnWriteBitmap empty;
   EXPECT_EQ(empty.begin(), empty.end());
}

TEST(CopyOnWriteBitmap, mutatingAViewDoesNotDisturbTheSource) {
   // A bitmap constructed from a pointer is copy-on-write: the first mutation must clone rather
   // than write through to the borrowed source.
   const roaring::Roaring source = multiContainer();
   CopyOnWriteBitmap under_test{&source};
   const roaring::Roaring other{5};
   under_test &= CopyOnWriteBitmap{&other};

   EXPECT_EQ(under_test.toRoaring(), roaring::Roaring{5});
   EXPECT_EQ(source, multiContainer());
}
