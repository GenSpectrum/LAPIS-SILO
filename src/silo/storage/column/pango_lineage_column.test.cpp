#include "silo/storage/column/pango_lineage_column.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(bugprone-unchecked-optional-access)

TEST(PangoLineageColumn, addingLineageAndThenSublineageFiltersCorrectly) {
   silo::common::BidirectionalMap<silo::common::UnaliasedPangoLineage> lookup;
   silo::PangoLineageAliasLookup alias_key = silo::PangoLineageAliasLookup::readFromFile(
      "testBaseData/exampleDataset/pangolineage_alias.json"
   );
   silo::storage::column::PangoLineageColumnPartition under_test(alias_key, lookup);

   under_test.insert({"A.1.2"});
   under_test.insert({"A.1.2"});
   under_test.insert({"A.1.2.3"});
   under_test.insert({"A.1.2.3.4"});
   under_test.insert({"A.1.2"});

   EXPECT_EQ(*under_test.filter({"A.1.2"}).value(), roaring::Roaring({0, 1, 4}));
   EXPECT_EQ(
      *under_test.filterIncludingSublineages({"A.1.2"}).value(), roaring::Roaring({0, 1, 2, 3, 4})
   );

   EXPECT_EQ(*under_test.filter({"A.1.2.3"}).value(), roaring::Roaring({2}));
   EXPECT_EQ(*under_test.filterIncludingSublineages({"A.1.2.3"}).value(), roaring::Roaring({2, 3}));
}

TEST(PangoLineageColumn, addingSublineageAndThenLineageFiltersCorrectly) {
   silo::common::BidirectionalMap<silo::common::UnaliasedPangoLineage> lookup;
   silo::PangoLineageAliasLookup alias_key = silo::PangoLineageAliasLookup::readFromFile(
      "testBaseData/exampleDataset/pangolineage_alias.json"
   );
   silo::storage::column::PangoLineageColumnPartition under_test(alias_key, lookup);

   under_test.insert({"A.1.2.3"});
   under_test.insert({"A.1.2.3"});
   under_test.insert({"A.1"});
   under_test.insert({"A.1.2"});
   under_test.insert({"A.1.2.3"});

   EXPECT_EQ(*under_test.filter({"A.1.2"}).value(), roaring::Roaring({3}));
   EXPECT_EQ(
      *under_test.filterIncludingSublineages({"A.1.2"}).value(), roaring::Roaring({0, 1, 3, 4})
   );

   EXPECT_EQ(*under_test.filter({"A.1.2.3"}).value(), roaring::Roaring({0, 1, 4}));
   EXPECT_EQ(
      *under_test.filterIncludingSublineages({"A.1.2.3"}).value(), roaring::Roaring({0, 1, 4})
   );
}

TEST(PangoLineageColumn, queryParentLineageThatWasNeverInserted) {
   silo::common::BidirectionalMap<silo::common::UnaliasedPangoLineage> lookup;
   silo::PangoLineageAliasLookup alias_key = silo::PangoLineageAliasLookup::readFromFile(
      "testBaseData/exampleDataset/pangolineage_alias.json"
   );
   silo::storage::column::PangoLineageColumnPartition under_test(alias_key, lookup);

   under_test.insert({"A.1.2.3"});
   under_test.insert({"A.1.2.3"});
   under_test.insert({"A.2"});
   under_test.insert({"A.1.2"});

   EXPECT_EQ(under_test.filter({"A.1"}), std::nullopt);
   EXPECT_EQ(*under_test.filterIncludingSublineages({"A.1"}).value(), roaring::Roaring({0, 1, 3}));
}

// NOLINTEND(bugprone-unchecked-optional-access)
