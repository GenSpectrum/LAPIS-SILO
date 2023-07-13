#include "silo/common/bidirectional_map.h"

#include <gtest/gtest.h>

#include "silo/common/pango_lineage.h"

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

TEST(BidirectionalMap, correctPangoLineageDict) {
   BidirectionalMap<UnaliasedPangoLineage> under_test;
   EXPECT_EQ(under_test.getId(UnaliasedPangoLineage{"Not in dict"}), std::nullopt);

   EXPECT_EQ(under_test.getOrCreateId(UnaliasedPangoLineage{"Now in dict"}), 0);
   EXPECT_EQ(under_test.getOrCreateId(UnaliasedPangoLineage{"Now in dict"}), 0);
   EXPECT_EQ(under_test.getOrCreateId(UnaliasedPangoLineage{"Second in dict"}), 1);

   EXPECT_EQ(under_test.getId(UnaliasedPangoLineage{"Now in dict"}), 0);
   EXPECT_EQ(under_test.getId(UnaliasedPangoLineage{"Still not in dict"}), std::nullopt);
   EXPECT_EQ(under_test.getId(UnaliasedPangoLineage{"Second in dict"}), 1);

   EXPECT_EQ(under_test.getValue(0), UnaliasedPangoLineage{"Now in dict"});
   EXPECT_EQ(under_test.getValue(1), UnaliasedPangoLineage{"Second in dict"});
}

}  // namespace silo::common