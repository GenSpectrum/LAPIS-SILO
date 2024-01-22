#include "silo/storage/column/pango_lineage_column.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(bugprone-unchecked-optional-access)

TEST(PangoLineageColumn, addingLineageAndThenSublineageFiltersCorrectly) {
   silo::common::BidirectionalMap<silo::common::UnaliasedPangoLineage> lookup_unaliased;
   silo::common::BidirectionalMap<silo::common::AliasedPangoLineage> lookup_aliased;
   auto alias_key = silo::PangoLineageAliasLookup::readFromFile(
      "testBaseData/exampleDataset/pangolineage_alias.json"
   );
   auto under_test = silo::storage::column::PangoLineageColumnPartition(
      alias_key, lookup_unaliased, lookup_aliased
   );

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
   silo::common::BidirectionalMap<silo::common::UnaliasedPangoLineage> lookup_unaliased;
   silo::common::BidirectionalMap<silo::common::AliasedPangoLineage> lookup_aliased;
   auto alias_key = silo::PangoLineageAliasLookup::readFromFile(
      "testBaseData/exampleDataset/pangolineage_alias.json"
   );
   auto under_test = silo::storage::column::PangoLineageColumnPartition(
      alias_key, lookup_unaliased, lookup_aliased
   );

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
   silo::common::BidirectionalMap<silo::common::UnaliasedPangoLineage> lookup_unaliased;
   silo::common::BidirectionalMap<silo::common::AliasedPangoLineage> lookup_aliased;
   auto alias_key = silo::PangoLineageAliasLookup::readFromFile(
      "testBaseData/exampleDataset/pangolineage_alias.json"
   );
   auto under_test = silo::storage::column::PangoLineageColumnPartition(
      alias_key, lookup_unaliased, lookup_aliased
   );

   under_test.insert({"A.1.2.3"});
   under_test.insert({"A.1.2.3"});
   under_test.insert({"A.2"});
   under_test.insert({"A.1.2"});

   EXPECT_EQ(under_test.filter({"A.1"}), std::nullopt);
   EXPECT_EQ(*under_test.filterIncludingSublineages({"A.1"}).value(), roaring::Roaring({0, 1, 3}));
}

TEST(PangoLineageColumnPartition, returnsAliasedLookupValue) {
   silo::common::BidirectionalMap<silo::common::UnaliasedPangoLineage> lookup_unaliased;
   silo::common::BidirectionalMap<silo::common::AliasedPangoLineage> lookup_aliased;
   auto alias_key = silo::PangoLineageAliasLookup::readFromFile(
      "testBaseData/exampleDataset/pangolineage_alias.json"
   );
   auto under_test = silo::storage::column::PangoLineageColumnPartition(
      alias_key, lookup_unaliased, lookup_aliased
   );

   under_test.insert({"B.1.1"});
   under_test.insert({"C"});
   under_test.insert({"B.1.1.1.1"});

   EXPECT_EQ(under_test.lookupAliasedValue(0), silo::common::AliasedPangoLineage("B"));
   EXPECT_EQ(under_test.lookupAliasedValue(1), silo::common::AliasedPangoLineage("B.1"));
   EXPECT_EQ(under_test.lookupAliasedValue(2), silo::common::AliasedPangoLineage("B.1.1"));
   EXPECT_EQ(under_test.lookupAliasedValue(3), silo::common::AliasedPangoLineage("B.1.1.1"));
   EXPECT_EQ(under_test.lookupAliasedValue(4), silo::common::AliasedPangoLineage("C.1"));

   EXPECT_EQ(under_test.lookupUnaliasedValue(0), silo::common::UnaliasedPangoLineage("B"));
   EXPECT_EQ(under_test.lookupUnaliasedValue(1), silo::common::UnaliasedPangoLineage("B.1"));
   EXPECT_EQ(under_test.lookupUnaliasedValue(2), silo::common::UnaliasedPangoLineage("B.1.1"));
   EXPECT_EQ(under_test.lookupUnaliasedValue(3), silo::common::UnaliasedPangoLineage("B.1.1.1"));
   EXPECT_EQ(under_test.lookupUnaliasedValue(4), silo::common::UnaliasedPangoLineage("B.1.1.1.1"));
}
// NOLINTEND(bugprone-unchecked-optional-access)
