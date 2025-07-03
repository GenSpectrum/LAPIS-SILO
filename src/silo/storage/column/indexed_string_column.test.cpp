#include "silo/storage/column/indexed_string_column.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "silo/preprocessing/preprocessing_exception.h"

using silo::common::LineageTreeAndIdMap;
using silo::common::RecombinantEdgeFollowingMode;
using silo::preprocessing::LineageDefinitionFile;
using silo::storage::column::IndexedStringColumnMetadata;
using silo::storage::column::IndexedStringColumnPartition;

// NOLINTBEGIN(bugprone-unchecked-optional-access)

TEST(IndexedStringColumnPartition, shouldReturnTheCorrectFilteredValues) {
   IndexedStringColumnMetadata column_metadata("some_column");
   IndexedStringColumnPartition under_test{&column_metadata};

   under_test.insert("value 1");
   under_test.insert("value 2");
   under_test.insert("value 2");
   under_test.insert("value 3");
   under_test.insert("value 1");

   const auto result1 = under_test.filter("value 1");
   ASSERT_EQ(*result1.value(), roaring::Roaring({0, 4}));

   const auto result2 = under_test.filter("value 2");
   ASSERT_EQ(*result2.value(), roaring::Roaring({1, 2}));

   const auto result3 = under_test.filter("value that does not exist");
   ASSERT_EQ(result3, std::nullopt);
}

TEST(IndexedStringColumnPartition, insertValuesToPartition) {
   IndexedStringColumnMetadata column_metadata("some_column");
   IndexedStringColumnPartition under_test{&column_metadata};

   under_test.insert("value 1");
   under_test.insert("value 2");
   under_test.insert("value 2");
   under_test.insert("value 3");
   under_test.insert("value 1");

   EXPECT_EQ(under_test.getValues()[0], 0U);
   EXPECT_EQ(under_test.getValues()[1], 1U);
   EXPECT_EQ(under_test.getValues()[2], 1U);
   EXPECT_EQ(under_test.getValues()[3], 2U);
   EXPECT_EQ(under_test.getValues()[4], 0U);

   EXPECT_EQ(under_test.lookupValue(0U), "value 1");
   EXPECT_EQ(under_test.lookupValue(1U), "value 2");
   EXPECT_EQ(under_test.lookupValue(2U), "value 3");
}

TEST(IndexedStringColumnPartition, addingLineageAndThenSublineageFiltersCorrectly) {
   auto lineage_definitions = LineageTreeAndIdMap::fromLineageDefinitionFilePath(
      "testBaseData/exampleDataset/lineage_definitions.yaml"
   );
   IndexedStringColumnMetadata column_metadata("some_column", lineage_definitions);
   IndexedStringColumnPartition under_test{&column_metadata};

   under_test.insert({"BA.1.1"});
   under_test.insert({"BA.1.1"});
   under_test.insert({"BA.1.1.1"});
   under_test.insert({"BA.1.1.1.1"});
   under_test.insert({"BA.1.1"});

   EXPECT_EQ(*under_test.filter({"BA.1.1"}).value(), roaring::Roaring({0, 1, 4}));
   EXPECT_EQ(
      *under_test.getLineageIndex()
          ->filterIncludingSublineages(
             under_test.getValueId("BA.1.1").value(), RecombinantEdgeFollowingMode::DO_NOT_FOLLOW
          )
          .value(),
      roaring::Roaring({0, 1, 2, 3, 4})
   );

   EXPECT_EQ(*under_test.filter({"BA.1.1.1"}).value(), roaring::Roaring({2}));
   EXPECT_EQ(
      *under_test.getLineageIndex()
          ->filterIncludingSublineages(
             under_test.getValueId("BA.1.1.1").value(), RecombinantEdgeFollowingMode::DO_NOT_FOLLOW
          )
          .value(),
      roaring::Roaring({2, 3})
   );
}

TEST(IndexedStringColumnPartition, addingSublineageAndThenLineageFiltersCorrectly) {
   auto lineage_definitions = LineageTreeAndIdMap::fromLineageDefinitionFilePath(
      "testBaseData/exampleDataset/lineage_definitions.yaml"
   );
   IndexedStringColumnMetadata column_metadata("some_column", lineage_definitions);
   IndexedStringColumnPartition under_test{&column_metadata};

   under_test.insert({"BA.1.1.1"});
   under_test.insert({"BA.1.1.1"});
   under_test.insert({"BA.1"});
   under_test.insert({"BA.1.1"});
   under_test.insert({"BA.1.1.1"});

   EXPECT_EQ(*under_test.filter({"BA.1.1"}).value(), roaring::Roaring({3}));
   EXPECT_EQ(under_test.filter({"B.1.1.529.1.1"}), std::nullopt);
   EXPECT_EQ(
      *under_test.getLineageIndex()
          ->filterExcludingSublineages(under_test.getValueId("B.1.1.529.1.1").value())
          .value(),
      roaring::Roaring({3})
   );
   EXPECT_EQ(
      *under_test.getLineageIndex()
          ->filterIncludingSublineages(
             under_test.getValueId("BA.1.1").value(), RecombinantEdgeFollowingMode::DO_NOT_FOLLOW
          )
          .value(),
      roaring::Roaring({0, 1, 3, 4})
   );

   EXPECT_EQ(
      *under_test.getLineageIndex()
          ->filterIncludingSublineages(
             under_test.getValueId("BA.1.1.1").value(), RecombinantEdgeFollowingMode::DO_NOT_FOLLOW
          )
          .value(),
      roaring::Roaring({0, 1, 4})
   );
   EXPECT_EQ(
      *under_test.getLineageIndex()
          ->filterIncludingSublineages(
             under_test.getValueId("BA.1.1.1").value(), RecombinantEdgeFollowingMode::DO_NOT_FOLLOW
          )
          .value(),
      roaring::Roaring({0, 1, 4})
   );
}

TEST(IndexedStringColumnPartition, queryParentLineageThatWasNeverInserted) {
   auto lineage_definitions = LineageTreeAndIdMap::fromLineageDefinitionFilePath(
      "testBaseData/exampleDataset/lineage_definitions.yaml"
   );
   IndexedStringColumnMetadata column_metadata("some_column", lineage_definitions);
   IndexedStringColumnPartition under_test{&column_metadata};

   under_test.insert({"BA.1.1.1"});
   under_test.insert({"BA.1.1.1"});
   under_test.insert({"BA.2"});
   under_test.insert({"BA.1.1"});

   EXPECT_EQ(
      under_test.getLineageIndex()->filterExcludingSublineages(under_test.getValueId("BA.1").value()
      ),
      std::nullopt
   );
   EXPECT_EQ(
      *under_test.getLineageIndex()
          ->filterIncludingSublineages(
             under_test.getValueId("BA.1").value(), RecombinantEdgeFollowingMode::DO_NOT_FOLLOW
          )
          .value(),
      roaring::Roaring({0, 1, 3})
   );
}

TEST(IndexedStringColumnPartition, errorWhenInsertingIncorrectLineages) {
   auto lineage_definitions =
      LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAMLString(R"(
A: {}
A.1:
  parents: ["A"]
)"));
   IndexedStringColumnMetadata column_metadata("some_column", lineage_definitions);
   IndexedStringColumnPartition under_test{&column_metadata};
   under_test.insert({"A"});
   EXPECT_THAT(
      [&]() { under_test.insert({"A.2"}); },
      ThrowsMessage<silo::preprocessing::PreprocessingException>(
         ::testing::HasSubstr("The value 'A.2' is not a valid lineage value for column "
                              "'some_column'. Is your lineage definition file outdated?")
      )
   );
}

// NOLINTEND(bugprone-unchecked-optional-access)
