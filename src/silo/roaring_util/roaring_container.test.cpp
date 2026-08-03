#include "silo/roaring_util/roaring_container.h"

#include <sstream>

#include <gtest/gtest.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <roaring/roaring.hh>

#include "silo/roaring_util/bitmap_builder.h"

using silo::roaring_util::BitmapBuilderByContainer;
using silo::roaring_util::RoaringContainer;

namespace {

// Reconstructs the set of low-16-bit values held by a container by materializing it into a
// single-container roaring bitmap.
roaring::Roaring toRoaring(const RoaringContainer& container) {
   BitmapBuilderByContainer builder;
   builder.addContainer(0, container.rawContainer(), container.getTypecode());
   return std::move(builder).getBitmap();
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
