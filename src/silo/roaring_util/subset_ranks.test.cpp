#include "silo/roaring_util/subset_ranks.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

using silo::roaring_util::roaringSubsetRanks;

namespace {

class RoaringSubsetRanksTest : public ::testing::Test {
  protected:
   // Helper to create a container from a vector of values
   std::pair<roaring::internal::container_t*, uint8_t> createContainer(
      const std::vector<uint16_t>& values
   ) {
      assert(not values.empty());
      roaring::Roaring r;
      for (uint16_t val : values) {
         r.add(val);
      }
      auto cloned_container = roaring::internal::container_clone(
         r.roaring.high_low_container.containers[0], r.roaring.high_low_container.typecodes[0]
      );
      return {cloned_container, r.roaring.high_low_container.typecodes[0]};
   }
};

}  // namespace

TEST_F(RoaringSubsetRanksTest, BasicIntersection) {
   // Test the example from the comments: A={3,4,5,7,9}, B={4,5,6,9}
   // Expected ranks: 4->2, 5->3, 9->5

   auto [c_a, type_a] = createContainer({3, 4, 5, 7, 9});
   auto [c_b, type_b] = createContainer({4, 5, 6, 9});
   uint32_t base = 0;

   auto result = roaringSubsetRanks(c_a, type_a, c_b, type_b, base);

   ASSERT_EQ(result.size(), 3);
   EXPECT_EQ(result[0], 2);  // rank of 4 in A
   EXPECT_EQ(result[1], 3);  // rank of 5 in A
   EXPECT_EQ(result[2], 5);  // rank of 9 in A

   // Cleanup
   roaring::internal::container_free(c_a, type_a);
   roaring::internal::container_free(c_b, type_b);
}

TEST_F(RoaringSubsetRanksTest, EmptyIntersection) {
   // Test with no common elements: A={1,2,3}, B={4,5,6}
   // Expected: empty result

   auto [c_a, type_a] = createContainer({1, 2, 3});
   auto [c_b, type_b] = createContainer({4, 5, 6});
   uint32_t base = 0;

   auto result = roaringSubsetRanks(c_a, type_a, c_b, type_b, base);

   EXPECT_TRUE(result.empty());

   // Cleanup
   roaring::internal::container_free(c_a, type_a);
   roaring::internal::container_free(c_b, type_b);
}

TEST_F(RoaringSubsetRanksTest, IdenticalSets) {
   // Test when A and B are identical: A=B={10,20,30,40}
   // Expected ranks: 10->1, 20->2, 30->3, 40->4

   auto [c_a, type_a] = createContainer({10, 20, 30, 40});
   auto [c_b, type_b] = createContainer({10, 20, 30, 40});
   uint32_t base = 0;

   auto result = roaringSubsetRanks(c_a, type_a, c_b, type_b, base);

   ASSERT_EQ(result.size(), 4);
   EXPECT_EQ(result[0], 1);  // rank of 10
   EXPECT_EQ(result[1], 2);  // rank of 20
   EXPECT_EQ(result[2], 3);  // rank of 30
   EXPECT_EQ(result[3], 4);  // rank of 40

   // Cleanup
   roaring::internal::container_free(c_a, type_a);
   roaring::internal::container_free(c_b, type_b);
}

TEST_F(RoaringSubsetRanksTest, SingleElementIntersection) {
   // Test with single common element: A={1,5,10,15,20}, B={10}
   // Expected: rank of 10 in A is 3

   auto [c_a, type_a] = createContainer({1, 5, 10, 15, 20});
   auto [c_b, type_b] = createContainer({10});
   uint32_t base = 0;

   auto result = roaringSubsetRanks(c_a, type_a, c_b, type_b, base);

   ASSERT_EQ(result.size(), 1);
   EXPECT_EQ(result[0], 3);  // rank of 10 in A

   // Cleanup
   roaring::internal::container_free(c_a, type_a);
   roaring::internal::container_free(c_b, type_b);
}
