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
}
