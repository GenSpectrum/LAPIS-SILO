#include "silo/storage/column/indexed_string_column.h"

#include <expected>
#include <initializer_list>
#include <string>
#include <string_view>

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

using silo::common::LineageTreeAndIdMap;
using silo::common::RecombinantEdgeFollowingMode;
using silo::preprocessing::LineageDefinitionFile;
using silo::storage::column::IndexedStringColumn;
using silo::storage::column::IndexedStringColumnMetadata;

namespace {
// Buffers the values into a chunk and appends it to the column.
[[nodiscard]] std::expected<void, std::string> appendIndexedValues(
   IndexedStringColumn& column,
   std::initializer_list<std::string_view> values
) {
   IndexedStringColumn::Builder builder;
   for (const auto& value : values) {
      builder.insert(value);
   }
   return column.appendChunk(builder.finalize());
}
}  // namespace

// NOLINTBEGIN(bugprone-unchecked-optional-access)

TEST(IndexedStringColumn, shouldReturnTheCorrectFilteredValues) {
   IndexedStringColumnMetadata column_metadata("some_column");
   IndexedStringColumn under_test{&column_metadata};

   ASSERT_TRUE(
      appendIndexedValues(under_test, {"value 1", "value 2", "value 2", "value 3", "value 1"})
         .has_value()
   );

   const auto result1 = under_test.filter("value 1");
   ASSERT_EQ(*result1.value(), roaring::Roaring({0, 4}));

   const auto result2 = under_test.filter("value 2");
   ASSERT_EQ(*result2.value(), roaring::Roaring({1, 2}));

   const auto result3 = under_test.filter("value that does not exist");
   ASSERT_EQ(result3, std::nullopt);
}

TEST(IndexedStringColumn, insertValuesToPartition) {
   IndexedStringColumnMetadata column_metadata("some_column");
   IndexedStringColumn under_test{&column_metadata};

   ASSERT_TRUE(
      appendIndexedValues(under_test, {"value 1", "value 2", "value 2", "value 3", "value 1"})
         .has_value()
   );

   EXPECT_EQ(under_test.getValue(0), 0U);
   EXPECT_EQ(under_test.getValue(1), 1U);
   EXPECT_EQ(under_test.getValue(2), 1U);
   EXPECT_EQ(under_test.getValue(3), 2U);
   EXPECT_EQ(under_test.getValue(4), 0U);

   EXPECT_EQ(under_test.lookupValue(0U), "value 1");
   EXPECT_EQ(under_test.lookupValue(1U), "value 2");
   EXPECT_EQ(under_test.lookupValue(2U), "value 3");
}

TEST(IndexedStringColumn, addingLineageAndThenSublineageFiltersCorrectly) {
   auto lineage_definition = LineageTreeAndIdMap::fromLineageDefinitionFilePath(
      "testBaseData/exampleDataset/lineage_definition.yaml"
   );
   IndexedStringColumnMetadata column_metadata("some_column", lineage_definition, false);
   IndexedStringColumn under_test{&column_metadata};

   ASSERT_TRUE(
      appendIndexedValues(under_test, {"BA.1.1", "BA.1.1", "BA.1.1.1", "BA.1.1.1.1", "BA.1.1"})
         .has_value()
   );

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

TEST(IndexedStringColumn, addingSublineageAndThenLineageFiltersCorrectly) {
   auto lineage_definition = LineageTreeAndIdMap::fromLineageDefinitionFilePath(
      "testBaseData/exampleDataset/lineage_definition.yaml"
   );
   IndexedStringColumnMetadata column_metadata("some_column", lineage_definition, false);
   IndexedStringColumn under_test{&column_metadata};

   ASSERT_TRUE(
      appendIndexedValues(under_test, {"BA.1.1.1", "BA.1.1.1", "BA.1", "BA.1.1", "BA.1.1.1"})
         .has_value()
   );

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

TEST(IndexedStringColumn, queryParentLineageThatWasNeverInserted) {
   auto lineage_definition = LineageTreeAndIdMap::fromLineageDefinitionFilePath(
      "testBaseData/exampleDataset/lineage_definition.yaml"
   );
   IndexedStringColumnMetadata column_metadata("some_column", lineage_definition, false);
   IndexedStringColumn under_test{&column_metadata};

   ASSERT_TRUE(
      appendIndexedValues(under_test, {"BA.1.1.1", "BA.1.1.1", "BA.2", "BA.1.1"}).has_value()
   );

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

TEST(IndexedStringColumn, errorWhenInsertingIncorrectLineages) {
   auto lineage_definition =
      LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAMLString(R"(
A: {}
A.1:
  parents: ["A"]
)"));
   IndexedStringColumnMetadata column_metadata("some_column", lineage_definition, false);
   IndexedStringColumn under_test{&column_metadata};
   ASSERT_TRUE(appendIndexedValues(under_test, {"A"}).has_value());
   auto success = appendIndexedValues(under_test, {"A.2"});
   ASSERT_FALSE(success.has_value());
   ASSERT_EQ(
      success.error(),
      "The value 'A.2' is not a valid lineage value for column 'some_column'. "
      "Is your lineage definition file outdated?"
   );
}

TEST(IndexedStringColumn, ignoringErrorWhenInsertingIncorrectLineagesIfSpecified) {
   auto lineage_definition =
      LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAMLString(R"(
A: {}
A.1:
  parents: ["A"]
)"));
   IndexedStringColumnMetadata column_metadata("some_column", lineage_definition, true);
   IndexedStringColumn under_test{&column_metadata};
   ASSERT_TRUE(appendIndexedValues(under_test, {"A", "not in the lineage hierarchy"}).has_value());
   EXPECT_EQ(
      *under_test.getLineageIndex()
          ->filterIncludingSublineages(
             under_test.getValueId("A").value(), RecombinantEdgeFollowingMode::DO_NOT_FOLLOW
          )
          .value(),
      roaring::Roaring({0})
   );
}

// NOLINTEND(bugprone-unchecked-optional-access)
