#include "rhydb/query_engine/copy_on_write_bitmap.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

#include "rhydb/roaring_util/roaring_container.h"

using rhydb::query_engine::CopyOnWriteBitmap;
using rhydb::roaring_util::RoaringContainer;
using rhydb::roaring_util::RoaringContainerView;

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

TEST(CopyOnWriteBitmap, fastUnionEdgeCases) {
   EXPECT_TRUE(CopyOnWriteBitmap::fastUnion({}).toRoaring().isEmpty());

   // Inputs that contribute no containers at all must not upset the merge.
   const roaring::Roaring source = multiContainer();
   std::vector<CopyOnWriteBitmap> with_empties;
   with_empties.emplace_back();
   with_empties.emplace_back(&source);
   with_empties.emplace_back();
   EXPECT_EQ(CopyOnWriteBitmap::fastUnion(with_empties).toRoaring(), source);

   std::vector<CopyOnWriteBitmap> only_empties;
   only_empties.emplace_back();
   only_empties.emplace_back();
   EXPECT_TRUE(CopyOnWriteBitmap::fastUnion(only_empties).toRoaring().isEmpty());

   std::vector<CopyOnWriteBitmap> single;
   single.emplace_back(&source);
   EXPECT_EQ(CopyOnWriteBitmap::fastUnion(single).toRoaring(), source);
}

TEST(CopyOnWriteBitmap, fastUnionResultOutlivesItsInputs) {
   // The inputs of a union are typically temporaries (the evaluated children of a Union operator),
   // so the result must own its containers rather than view them.
   const roaring::Roaring first{1, 5};
   const roaring::Roaring second{5, 9, (2U << 16) + 1};
   const roaring::Roaring expected = first | second;

   CopyOnWriteBitmap result;
   {
      std::vector<CopyOnWriteBitmap> bitmaps;
      bitmaps.emplace_back(roaring::Roaring{first});
      bitmaps.emplace_back(roaring::Roaring{second});
      result = CopyOnWriteBitmap::fastUnion(bitmaps);
   }
   EXPECT_EQ(result.toRoaring(), expected);
}

TEST(CopyOnWriteBitmap, fastUnionMergesManyInputsWithStaggeredKeyRanges) {
   // Inputs whose key ranges start and end at different points, so cursors enter and leave the
   // merge in a different order than they were added, and no single input spans every key.
   std::vector<roaring::Roaring> sources;
   for (uint32_t bitmap_index = 0; bitmap_index < 8; ++bitmap_index) {
      roaring::Roaring source;
      for (uint32_t key = bitmap_index; key < bitmap_index + 5; ++key) {
         // Dense enough to hit array, bitset and run container representations.
         source.addRange(
            (key << 16U) + bitmap_index, (key << 16U) + bitmap_index + 5000 + (key * 100)
         );
         source.add((key << 16U) + 60000);
      }
      source.runOptimize();
      sources.push_back(std::move(source));
   }

   roaring::Roaring expected;
   std::vector<CopyOnWriteBitmap> bitmaps;
   for (const auto& source : sources) {
      expected |= source;
      bitmaps.emplace_back(&source);
   }

   const auto united = CopyOnWriteBitmap::fastUnion(bitmaps);
   EXPECT_EQ(united.toRoaring(), expected);
   EXPECT_EQ(united.cardinality(), expected.cardinality());
}

TEST(CopyOnWriteBitmap, fastUnionIsOrderIndependentAndMatchesRepeatedOrEqual) {
   const roaring::Roaring first{1, 5, 100};
   const roaring::Roaring second = multiContainer();
   const roaring::Roaring third{5, (3U << 16) + 7, (4U << 16) + 1};
   const roaring::Roaring fourth{(4U << 16) + 1, (9U << 16)};

   const std::vector<const roaring::Roaring*> sources{&first, &second, &third, &fourth};

   roaring::Roaring expected;
   CopyOnWriteBitmap accumulated;
   for (const auto* source : sources) {
      expected |= *source;
      accumulated |= CopyOnWriteBitmap{source};
   }
   EXPECT_EQ(accumulated.toRoaring(), expected);

   // Whichever order the inputs arrive in, the merge must produce the same set.
   std::vector<size_t> order{0, 1, 2, 3};
   do {
      std::vector<CopyOnWriteBitmap> bitmaps;
      bitmaps.reserve(order.size());
      for (const size_t index : order) {
         bitmaps.emplace_back(sources[index]);
      }
      EXPECT_EQ(CopyOnWriteBitmap::fastUnion(bitmaps).toRoaring(), expected);
   } while (std::ranges::next_permutation(order).found);
}

TEST(CopyOnWriteBitmap, iteratingContainersReconstructsRowIdsInAscendingOrder) {
   const roaring::Roaring source = multiContainer();
   const CopyOnWriteBitmap under_test{&source};

   std::vector<uint32_t> iterated;
   for (const auto& [key, container_view] : under_test) {
      const uint32_t high = static_cast<uint32_t>(key) << 16U;
      for (const uint16_t low : container_view) {
         iterated.push_back(high | low);
      }
   }
   EXPECT_EQ(iterated, (std::vector<uint32_t>{1, 5, 100, (1U << 16) + 3, (3U << 16) + 7}));

   const CopyOnWriteBitmap empty;
   EXPECT_EQ(empty.begin(), empty.end());
}

TEST(CopyOnWriteBitmap, fromContainerViewsAssemblesAndOrsSharedKeys) {
   // Owning containers the views borrow from; they must outlive the assembled bitmap.
   auto block0 = RoaringContainer::withCapacity(4);
   block0.add(1);
   block0.add(5);
   auto block1_first = RoaringContainer::withCapacity(4);
   block1_first.add(3);
   auto block1_second = RoaringContainer::withCapacity(4);
   block1_second.add(9);
   auto block3 = RoaringContainer::withCapacity(4);
   block3.add(7);

   // Deliberately out of key order, with key 1 appearing twice (must be OR-ed).
   std::vector<std::pair<uint16_t, RoaringContainerView>> views;
   views.emplace_back(3, RoaringContainerView{block3});
   views.emplace_back(1, RoaringContainerView{block1_first});
   views.emplace_back(0, RoaringContainerView{block0});
   views.emplace_back(1, RoaringContainerView{block1_second});

   const CopyOnWriteBitmap under_test = CopyOnWriteBitmap::fromContainerViews(std::move(views));

   roaring::Roaring expected{1, 5};
   expected.add((1U << 16) + 3);
   expected.add((1U << 16) + 9);
   expected.add((3U << 16) + 7);
   EXPECT_EQ(under_test.toRoaring(), expected);
}

TEST(CopyOnWriteBitmap, copyOfOwningBitmapIsIndependent) {
   // An owning bitmap (constructed by move) that is copied and then mutated must not affect the
   // original: each owning container is deep-cloned on copy.
   const roaring::Roaring expected = multiContainer();
   const CopyOnWriteBitmap original{multiContainer()};
   CopyOnWriteBitmap copy = original;

   const roaring::Roaring other{5};
   copy &= CopyOnWriteBitmap{&other};

   EXPECT_EQ(copy.toRoaring(), roaring::Roaring{5});
   EXPECT_EQ(original.toRoaring(), expected);
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
