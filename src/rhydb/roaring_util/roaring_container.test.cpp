#include "rhydb/roaring_util/roaring_container.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <vector>

#include <gtest/gtest.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <roaring/roaring.hh>

#include "rhydb/roaring_util/bitmap_builder.h"

using rhydb::roaring_util::BitmapBuilderByContainer;
using rhydb::roaring_util::RoaringContainer;
using rhydb::roaring_util::RoaringContainerView;

namespace {

// Reconstructs the set of low-16-bit values held by a container by materializing it into a
// single-container roaring bitmap.
roaring::Roaring toRoaring(const RoaringContainer& container) {
   BitmapBuilderByContainer builder;
   builder.addContainer(0, container.rawContainer(), container.getTypecode());
   return std::move(builder).getBitmap();
}

// Collects the low-16-bit values yielded by iterating a view, in iteration order.
std::vector<uint16_t> iterate(const RoaringContainerView& view) {
   std::vector<uint16_t> values;
   for (const uint16_t value : view) {
      values.push_back(value);
   }
   return values;
}

// Builds an owning container holding exactly `values`. An empty list yields an empty container
// (cardinality 0), which the set-algebra tests use to exercise the empty-operand paths.
RoaringContainer makeContainer(std::initializer_list<uint16_t> values) {
   auto container =
      RoaringContainer::withCapacity(static_cast<int32_t>(std::max(values.size(), std::size_t{1})));
   for (const uint16_t value : values) {
      container.add(value);
   }
   return container;
}

}  // namespace

TEST(RoaringContainer, addAndCardinality) {
   auto container = RoaringContainer::withCapacity(4);
   EXPECT_TRUE(container.empty());

   container.add(3);
   container.add(7);
   container.add(9);

   EXPECT_EQ(container.getCardinality(), 3);
   EXPECT_FALSE(container.empty());
   EXPECT_EQ(toRoaring(container), (roaring::Roaring{3, 7, 9}));
}

TEST(RoaringContainer, growsFromArrayToBitset) {
   // A small capacity starts as an array container and must transparently grow past the array
   // container limit into a bitset as values are added.
   auto container = RoaringContainer::withCapacity(1);
   const uint16_t count = 20000;
   for (uint16_t value = 0; value < count; ++value) {
      container.add(value);
   }
   EXPECT_EQ(container.getCardinality(), count);
   roaring::Roaring expected;
   expected.addRange(0, count);
   EXPECT_EQ(toRoaring(container), expected);
}

TEST(RoaringContainer, clonedFromIsIndependent) {
   auto source = RoaringContainer::withCapacity(4);
   source.add(1);
   source.add(2);

   auto clone = RoaringContainer::clonedFrom(source.rawContainer(), source.getTypecode());
   EXPECT_EQ(clone.getCardinality(), 2);

   // Mutating the clone must not affect the source.
   clone.add(3);
   EXPECT_EQ(clone.getCardinality(), 3);
   EXPECT_EQ(source.getCardinality(), 2);
}

TEST(RoaringContainer, moveTransfersOwnership) {
   auto container = RoaringContainer::withCapacity(4);
   container.add(5);

   auto moved = std::move(container);
   EXPECT_EQ(moved.getCardinality(), 1);
   EXPECT_EQ(toRoaring(moved), roaring::Roaring{5});
}

TEST(RoaringContainer, runOptimizeAndShrinkPreservesContents) {
   auto container = RoaringContainer::withCapacity(4);
   for (uint16_t value = 100; value < 200; ++value) {
      container.add(value);
   }
   const auto before = toRoaring(container);
   container.runOptimizeAndShrink();
   EXPECT_EQ(container.getCardinality(), 100);
   EXPECT_EQ(toRoaring(container), before);
}

TEST(RoaringContainerView, mirrorsOwnerAndDoesNotFree) {
   auto owner = RoaringContainer::withCapacity(4);
   owner.add(3);
   owner.add(7);

   const RoaringContainerView view{owner};
   EXPECT_EQ(view.getCardinality(), 2);
   EXPECT_FALSE(view.empty());
   EXPECT_EQ(view.rawContainer(), owner.rawContainer());
   EXPECT_EQ(view.getTypecode(), owner.getTypecode());

   // The view is non-owning: when it goes out of scope the owner's container stays valid.
   {
      const RoaringContainerView scoped{owner};
      EXPECT_EQ(scoped.getCardinality(), 2);
   }
   EXPECT_EQ(toRoaring(owner), (roaring::Roaring{3, 7}));
}

TEST(RoaringContainerView, toOwningDeepCopies) {
   auto owner = RoaringContainer::withCapacity(4);
   owner.add(1);
   owner.add(2);

   const RoaringContainerView view{owner};
   RoaringContainer owned = view.toOwning();
   EXPECT_EQ(owned.getCardinality(), 2);

   // Mutating the deep copy must not affect the original owner.
   owned.add(3);
   EXPECT_EQ(owned.getCardinality(), 3);
   EXPECT_EQ(owner.getCardinality(), 2);
   EXPECT_EQ(toRoaring(owner), (roaring::Roaring{1, 2}));
}

TEST(RoaringContainerView, iteratesArrayContainerValuesAscending) {
   auto owner = RoaringContainer::withCapacity(4);
   owner.add(3);
   owner.add(7);
   owner.add(9);

   const RoaringContainerView view{owner};
   EXPECT_EQ(iterate(view), (std::vector<uint16_t>{3, 7, 9}));
}

TEST(RoaringContainerView, iteratesBitsetContainerValuesAscending) {
   // Enough values to force the array container to grow into a bitset container.
   auto owner = RoaringContainer::withCapacity(1);
   const uint16_t count = 20000;
   for (uint16_t value = 0; value < count; ++value) {
      owner.add(value);
   }
   ASSERT_EQ(owner.getTypecode(), BITSET_CONTAINER_TYPE);

   const auto values = iterate(RoaringContainerView{owner});
   ASSERT_EQ(values.size(), count);
   for (uint16_t value = 0; value < count; ++value) {
      EXPECT_EQ(values[value], value);
   }
}

TEST(RoaringContainerView, iteratesRunContainerValuesAscending) {
   // A dense contiguous range run-optimizes into a run container.
   auto owner = RoaringContainer::withCapacity(4);
   for (uint16_t value = 100; value < 200; ++value) {
      owner.add(value);
   }
   owner.runOptimizeAndShrink();
   ASSERT_EQ(owner.getTypecode(), RUN_CONTAINER_TYPE);

   const auto values = iterate(RoaringContainerView{owner});
   ASSERT_EQ(values.size(), 100);
   for (uint16_t offset = 0; offset < 100; ++offset) {
      EXPECT_EQ(values[offset], 100 + offset);
   }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(RoaringContainerView, iteratorsAtDifferentValuesOfOneRunCompareUnequal) {
   // A single contiguous run: every value of the container lives in run index 0, so an iterator
   // position is only distinguishable by its current value, not by the internal run index.
   auto owner = RoaringContainer::withCapacity(4);
   for (uint16_t value = 100; value < 200; ++value) {
      owner.add(value);
   }
   owner.runOptimizeAndShrink();
   ASSERT_EQ(owner.getTypecode(), RUN_CONTAINER_TYPE);

   const RoaringContainerView view{owner};
   auto first = view.begin();
   auto second = view.begin();
   ++second;

   ASSERT_EQ(*first, 100);
   ASSERT_EQ(*second, 101);
   EXPECT_NE(first, second);
   EXPECT_FALSE(first == second);

   // Advancing a copy must not move the original, and equality must hold again once the copy has
   // been caught up.
   ++first;
   EXPECT_EQ(first, second);
   EXPECT_EQ(*first, *second);

   // Every position within the run must be distinct from every other one.
   for (auto left = view.begin(); left != RoaringContainerView::end(); ++left) {
      for (auto right = view.begin(); right != RoaringContainerView::end(); ++right) {
         EXPECT_EQ(left == right, *left == *right);
      }
   }
}

TEST(RoaringContainerView, algorithmsTerminateCorrectlyWithinOneRun) {
   // Standard algorithms compare iterators for equality; if positions inside a run collapsed onto
   // one another they would stop early or miscount.
   auto owner = RoaringContainer::withCapacity(4);
   for (uint16_t value = 100; value < 200; ++value) {
      owner.add(value);
   }
   owner.runOptimizeAndShrink();
   ASSERT_EQ(owner.getTypecode(), RUN_CONTAINER_TYPE);

   const RoaringContainerView view{owner};
   EXPECT_EQ(std::distance(view.begin(), RoaringContainerView::end()), 100);

   const auto found = std::find(view.begin(), RoaringContainerView::end(), uint16_t{150});
   ASSERT_NE(found, RoaringContainerView::end());
   EXPECT_EQ(*found, 150);
   EXPECT_EQ(std::distance(view.begin(), found), 50);

   EXPECT_EQ(
      std::find(view.begin(), RoaringContainerView::end(), uint16_t{250}),
      RoaringContainerView::end()
   );
}

TEST(RoaringContainerView, emptyViewIteratesAsEmptyRange) {
   const auto owner = RoaringContainer::withCapacity(4);
   const RoaringContainerView view{owner};
   ASSERT_TRUE(view.empty());
   EXPECT_EQ(view.begin(), view.end());
   EXPECT_TRUE(iterate(view).empty());
}

TEST(RoaringContainer, serializationRoundTrip) {
   auto container = RoaringContainer::withCapacity(4);
   container.add(1);
   container.add(42);
   container.add(1000);
   const auto expected = toRoaring(container);

   std::stringstream stream;
   {
      boost::archive::binary_oarchive output_archive(stream);
      output_archive << container;
   }

   RoaringContainer restored;
   {
      boost::archive::binary_iarchive input_archive(stream);
      input_archive >> restored;
   }

   EXPECT_EQ(restored.getCardinality(), 3);
   EXPECT_EQ(toRoaring(restored), expected);
}

TEST(RoaringContainerSetAlgebra, intersectionKeepsCommonValues) {
   const auto lhs = makeContainer({1, 2, 3, 5, 8});
   const auto rhs = makeContainer({2, 3, 5, 7});

   const RoaringContainer result = RoaringContainerView{lhs} & RoaringContainerView{rhs};
   EXPECT_EQ(result.getCardinality(), 3);
   EXPECT_EQ(toRoaring(result), (roaring::Roaring{2, 3, 5}));
}

TEST(RoaringContainerSetAlgebra, intersectionOfDisjointContainersIsEmpty) {
   const auto lhs = makeContainer({1, 3, 5});
   const auto rhs = makeContainer({2, 4, 6});

   const RoaringContainer result = RoaringContainerView{lhs} & RoaringContainerView{rhs};
   EXPECT_TRUE(result.empty());
   EXPECT_EQ(result.getCardinality(), 0);
}

TEST(RoaringContainerSetAlgebra, intersectionWithEmptyOperandIsEmpty) {
   const auto values = makeContainer({1, 2, 3});
   const auto empty = makeContainer({});

   EXPECT_TRUE((RoaringContainerView{values} & RoaringContainerView{empty}).empty());
   EXPECT_TRUE((RoaringContainerView{empty} & RoaringContainerView{values}).empty());
}

TEST(RoaringContainerSetAlgebra, differenceRemovesRhsValues) {
   const auto lhs = makeContainer({1, 2, 3, 4, 5});
   const auto rhs = makeContainer({2, 4});

   const RoaringContainer result = RoaringContainerView{lhs} - RoaringContainerView{rhs};
   EXPECT_EQ(toRoaring(result), (roaring::Roaring{1, 3, 5}));
}

TEST(RoaringContainerSetAlgebra, differenceWithEmptyRhsCopiesLhs) {
   const auto lhs = makeContainer({1, 2, 3});
   const auto empty = makeContainer({});

   const RoaringContainer result = RoaringContainerView{lhs} - RoaringContainerView{empty};
   EXPECT_EQ(toRoaring(result), (roaring::Roaring{1, 2, 3}));
   // The result is an independent owning copy, not a borrow of `lhs`.
   EXPECT_NE(result.rawContainer(), lhs.rawContainer());
}

TEST(RoaringContainerSetAlgebra, differenceWithEmptyLhsIsEmpty) {
   const auto empty = makeContainer({});
   const auto rhs = makeContainer({1, 2, 3});

   EXPECT_TRUE((RoaringContainerView{empty} - RoaringContainerView{rhs}).empty());
}

TEST(RoaringContainerSetAlgebra, differenceOfEqualContainersIsEmpty) {
   // The C-API difference produces an empty container here; the "no empty containers" invariant
   // requires it to come back as an empty owning container, not a kept zero-cardinality one.
   const auto values = makeContainer({1, 2, 3});

   const RoaringContainer result = RoaringContainerView{values} - RoaringContainerView{values};
   EXPECT_TRUE(result.empty());
   EXPECT_EQ(result.getCardinality(), 0);
}

TEST(RoaringContainerSetAlgebra, unionMergesValues) {
   const auto lhs = makeContainer({1, 3, 5});
   const auto rhs = makeContainer({2, 4, 6});

   const RoaringContainer result = RoaringContainerView{lhs} | RoaringContainerView{rhs};
   EXPECT_EQ(toRoaring(result), (roaring::Roaring{1, 2, 3, 4, 5, 6}));
}

TEST(RoaringContainerSetAlgebra, unionWithEmptyOperandsReturnsTheOther) {
   const auto values = makeContainer({1, 2, 3});
   const auto empty = makeContainer({});

   EXPECT_EQ(
      toRoaring(RoaringContainerView{empty} | RoaringContainerView{values}),
      (roaring::Roaring{1, 2, 3})
   );
   EXPECT_EQ(
      toRoaring(RoaringContainerView{values} | RoaringContainerView{empty}),
      (roaring::Roaring{1, 2, 3})
   );
   EXPECT_TRUE((RoaringContainerView{empty} | RoaringContainerView{empty}).empty());
}

TEST(RoaringContainerSetAlgebra, orAssignAccumulatesInPlace) {
   RoaringContainer accumulator = makeContainer({1, 2});
   const auto addend = makeContainer({2, 3, 4});

   accumulator |= RoaringContainerView{addend};
   EXPECT_EQ(toRoaring(accumulator), (roaring::Roaring{1, 2, 3, 4}));
}

TEST(RoaringContainerSetAlgebra, orAssignIntoEmptyAccumulatorTakesAddend) {
   RoaringContainer accumulator = makeContainer({});
   const auto addend = makeContainer({5, 6});

   accumulator |= RoaringContainerView{addend};
   EXPECT_EQ(toRoaring(accumulator), (roaring::Roaring{5, 6}));
}

TEST(RoaringContainerSetAlgebra, orAssignEmptyAddendLeavesAccumulatorUnchanged) {
   RoaringContainer accumulator = makeContainer({1, 2});
   const auto empty = makeContainer({});

   accumulator |= RoaringContainerView{empty};
   EXPECT_EQ(toRoaring(accumulator), (roaring::Roaring{1, 2}));
}
