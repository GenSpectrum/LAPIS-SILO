#include "silo/storage/column/pango_lineage_column.h"

#include <gtest/gtest.h>

TEST(PangoLineageColumn, addingLineageAndThenSublineageFiltersCorrectly) {
   silo::storage::column::PangoLineageColumn under_test;

   under_test.insert({"A.1.2"});
   under_test.insert({"A.1.2"});
   under_test.insert({"A.1.2.3"});
   under_test.insert({"A.1.2.3.4"});
   under_test.insert({"A.1.2"});

   EXPECT_EQ(under_test.filter({"A.1.2"}), roaring::Roaring({0, 1, 4}));
   EXPECT_EQ(under_test.filterIncludingSublineages({"A.1.2"}), roaring::Roaring({0, 1, 2, 3, 4}));

   EXPECT_EQ(under_test.filter({"A.1.2.3"}), roaring::Roaring({2}));
   EXPECT_EQ(under_test.filterIncludingSublineages({"A.1.2.3"}), roaring::Roaring({2, 3}));
}

TEST(PangoLineageColumn, addingSublineageAndThenLineageFiltersCorrectly) {
   silo::storage::column::PangoLineageColumn under_test;

   under_test.insert({"A.1.2.3"});
   under_test.insert({"A.1.2.3"});
   under_test.insert({"A.1"});
   under_test.insert({"A.1.2"});
   under_test.insert({"A.1.2.3"});

   EXPECT_EQ(under_test.filter({"A.1.2"}), roaring::Roaring({3}));
   EXPECT_EQ(under_test.filterIncludingSublineages({"A.1.2"}), roaring::Roaring({0, 1, 3, 4}));

   EXPECT_EQ(under_test.filter({"A.1.2.3"}), roaring::Roaring({0, 1, 4}));
   EXPECT_EQ(under_test.filterIncludingSublineages({"A.1.2.3"}), roaring::Roaring({0, 1, 4}));
}

TEST(PangoLineageColumn, queryParentLineageThatWasNeverInserted) {
   silo::storage::column::PangoLineageColumn under_test;

   under_test.insert({"A.1.2.3"});
   under_test.insert({"A.1.2.3"});
   under_test.insert({"A.2"});
   under_test.insert({"A.1.2"});

   EXPECT_EQ(under_test.filter({"A.1"}), roaring::Roaring());
   EXPECT_EQ(under_test.filterIncludingSublineages({"A.1"}), roaring::Roaring({0, 1, 3}));
}
