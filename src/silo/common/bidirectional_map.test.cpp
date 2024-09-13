#include "silo/common/bidirectional_map.h"

#include <gtest/gtest.h>

namespace silo::common {

TEST(BidirectionalMap, correctStdStringDict) {
   BidirectionalMap<std::string> under_test;
   EXPECT_EQ(under_test.getId("Not in dict"), std::nullopt);

   EXPECT_EQ(under_test.getOrCreateId("Now in dict"), 0);
   EXPECT_EQ(under_test.getOrCreateId("Now in dict"), 0);
   EXPECT_EQ(under_test.getOrCreateId("Second in dict"), 1);

   EXPECT_EQ(under_test.getId("Now in dict"), 0);
   EXPECT_EQ(under_test.getId("Still not in dict"), std::nullopt);
   EXPECT_EQ(under_test.getId("Second in dict"), 1);

   EXPECT_EQ(under_test.getValue(0), "Now in dict");
   EXPECT_EQ(under_test.getValue(1), "Second in dict");
}

TEST(BidirectionalMap, correctStdStringDictWithExplicitInitialization) {
   BidirectionalMap<std::string> under_test;
   EXPECT_EQ(under_test.getId(std::string{"Not in dict"}), std::nullopt);

   EXPECT_EQ(under_test.getOrCreateId(std::string{"Now in dict"}), 0);
   EXPECT_EQ(under_test.getOrCreateId(std::string{"Now in dict"}), 0);
   EXPECT_EQ(under_test.getOrCreateId(std::string{"Second in dict"}), 1);

   EXPECT_EQ(under_test.getId(std::string{"Now in dict"}), 0);
   EXPECT_EQ(under_test.getId(std::string{"Still not in dict"}), std::nullopt);
   EXPECT_EQ(under_test.getId(std::string{"Second in dict"}), 1);

   EXPECT_EQ(under_test.getValue(0), std::string{"Now in dict"});
   EXPECT_EQ(under_test.getValue(1), std::string{"Second in dict"});
}

}  // namespace silo::common