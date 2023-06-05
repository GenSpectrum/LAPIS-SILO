#include "silo/common/pango_lineage.h"

#include <gtest/gtest.h>

TEST(PangoLineage, isSublineageOf) {
   silo::common::PangoLineage const under_test{"A.1.2"};

   EXPECT_TRUE(under_test.isSublineageOf(silo::common::PangoLineage{"A.1"}));
   EXPECT_TRUE(under_test.isSublineageOf(silo::common::PangoLineage{"A"}));
   EXPECT_TRUE(under_test.isSublineageOf(silo::common::PangoLineage{"A.1.2"}));

   EXPECT_FALSE(under_test.isSublineageOf(silo::common::PangoLineage{"A.1.2.3"}));
   EXPECT_FALSE(under_test.isSublineageOf(silo::common::PangoLineage{"A.1.3"}));
   EXPECT_FALSE(under_test.isSublineageOf(silo::common::PangoLineage{"B.1.2"}));
   EXPECT_FALSE(under_test.isSublineageOf(silo::common::PangoLineage{"B.1.20"}));

   EXPECT_FALSE(silo::common::PangoLineage{"B.1.20"}.isSublineageOf(under_test));
}

TEST(PangoLineage, getParentLineages) {
   silo::common::PangoLineage const under_test{"A.1.23.4.513"};

   const std::vector<silo::common::PangoLineage> expected = {
      silo::common::PangoLineage{"A"},
      silo::common::PangoLineage{"A.1"},
      silo::common::PangoLineage{"A.1.23"},
      silo::common::PangoLineage{"A.1.23.4"},
      silo::common::PangoLineage{"A.1.23.4.513"},
   };

   EXPECT_EQ(under_test.getParentLineages(), expected);
}
