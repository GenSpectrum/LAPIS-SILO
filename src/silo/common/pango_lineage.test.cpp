#include "silo/common/pango_lineage.h"

#include <gtest/gtest.h>

TEST(UnaliasedPangoLineage, isSublineageOf) {
   silo::common::UnaliasedPangoLineage const under_test{"A.1.2"};

   EXPECT_TRUE(under_test.isSublineageOf(silo::common::UnaliasedPangoLineage{"A.1"}));
   EXPECT_TRUE(under_test.isSublineageOf(silo::common::UnaliasedPangoLineage{"A"}));
   EXPECT_TRUE(under_test.isSublineageOf(silo::common::UnaliasedPangoLineage{"A.1.2"}));

   EXPECT_FALSE(under_test.isSublineageOf(silo::common::UnaliasedPangoLineage{"A.1.2.3"}));
   EXPECT_FALSE(under_test.isSublineageOf(silo::common::UnaliasedPangoLineage{"A.1.3"}));
   EXPECT_FALSE(under_test.isSublineageOf(silo::common::UnaliasedPangoLineage{"B.1.2"}));
   EXPECT_FALSE(under_test.isSublineageOf(silo::common::UnaliasedPangoLineage{"B.1.20"}));

   EXPECT_FALSE(silo::common::UnaliasedPangoLineage{"B.1.20"}.isSublineageOf(under_test));
}

TEST(PangoLineage, getParentLineages) {
   silo::common::UnaliasedPangoLineage const under_test{"A.1.23.4.513"};

   const std::vector<silo::common::UnaliasedPangoLineage> expected = {
      silo::common::UnaliasedPangoLineage{"A"},
      silo::common::UnaliasedPangoLineage{"A.1"},
      silo::common::UnaliasedPangoLineage{"A.1.23"},
      silo::common::UnaliasedPangoLineage{"A.1.23.4"},
      silo::common::UnaliasedPangoLineage{"A.1.23.4.513"},
   };

   EXPECT_EQ(under_test.getParentLineages(), expected);
}
