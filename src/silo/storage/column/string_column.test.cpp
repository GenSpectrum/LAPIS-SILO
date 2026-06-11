#include "silo/storage/column/string_column.h"

#include <expected>
#include <initializer_list>
#include <string>
#include <string_view>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>

#include <gtest/gtest.h>
#include "silo/common/phylo_tree.h"
#include "silo/common/tree_node_id.h"

using silo::storage::column::StringColumn;
using silo::storage::column::StringColumnMetadata;

namespace {
// Buffers the values into a chunk and appends it to the column.
[[nodiscard]] std::expected<void, std::string> appendStringValues(
   StringColumn& column,
   std::initializer_list<std::string_view> values
) {
   StringColumn::Builder builder;
   for (const auto& value : values) {
      builder.insert(value);
   }
   return column.appendChunk(builder.finalize());
}
}  // namespace

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(StringColumn, rawInsertedValuesRequeried) {
   StringColumnMetadata metadata{"string_column"};
   StringColumn under_test(&metadata);

   SILO_ASSERT(appendStringValues(
                  under_test,
                  {"value 1",
                   "value 2",
                   "value 2",
                   "value 3",
                   "some string that is a little longer 1",
                   "value 1"}
   )
                  .has_value());

   EXPECT_EQ(under_test.getValueString(0), "value 1");
   EXPECT_EQ(under_test.getValueString(1), "value 2");
   EXPECT_EQ(under_test.getValueString(2), "value 2");
   EXPECT_EQ(under_test.getValueString(3), "value 3");
   EXPECT_EQ(under_test.getValueString(4), "some string that is a little longer 1");
   EXPECT_EQ(under_test.getValueString(5), "value 1");
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(StringColumn, serializationOfMetadataWorks) {
   auto phylo_tree = silo::common::PhyloTree::fromNewickString(
      "((CHILD2:0.5, CHILD3:1)CHILD:0.1, NOT_IN_DATASET:1.5)ROOT;"
   );
   StringColumnMetadata metadata{"string_column", std::move(phylo_tree)};
   StringColumn column(&metadata);

   SILO_ASSERT(appendStringValues(column, {"CHILD2", "CHILD3", "NOT_IN_TREE"}).has_value());

   std::ostringstream oss;
   boost::archive::binary_oarchive oarchive(oss);
   oarchive << metadata;

   std::istringstream iss(oss.str());
   boost::archive::binary_iarchive iarchive(iss);
   std::shared_ptr<StringColumnMetadata> under_test;
   iarchive >> under_test;

   auto node_dict = under_test->phylo_tree.value().nodes;

   EXPECT_EQ(node_dict.size(), 5);
   EXPECT_EQ(
      node_dict.at(silo::common::TreeNodeId{"CHILD2"})->parent, silo::common::TreeNodeId{"CHILD"}
   );
   EXPECT_EQ(node_dict.at(silo::common::TreeNodeId{"CHILD2"})->row_index, 0);
   EXPECT_EQ(
      node_dict.at(silo::common::TreeNodeId{"CHILD3"})->parent, silo::common::TreeNodeId{"CHILD"}
   );
   EXPECT_EQ(node_dict.at(silo::common::TreeNodeId{"CHILD3"})->row_index, 1);
   EXPECT_EQ(
      node_dict.at(silo::common::TreeNodeId{"CHILD"})->parent, silo::common::TreeNodeId{"ROOT"}
   );
   EXPECT_EQ(node_dict.at(silo::common::TreeNodeId{"CHILD"})->row_index, std::nullopt);
   EXPECT_EQ(
      node_dict.at(silo::common::TreeNodeId{"NOT_IN_DATASET"})->parent,
      silo::common::TreeNodeId{"ROOT"}
   );
   EXPECT_EQ(node_dict.at(silo::common::TreeNodeId{"NOT_IN_DATASET"})->row_index, std::nullopt);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(StringColumn, rawInsertedValuesWithPhyloTreeRequeried) {
   auto phylo_tree = silo::common::PhyloTree::fromNewickString(
      "((CHILD2:0.5, CHILD3:1)CHILD:0.1, NOT_IN_DATASET:1.5)ROOT;"
   );
   StringColumnMetadata metadata{"string_column", std::move(phylo_tree)};
   StringColumn under_test(&metadata);

   SILO_ASSERT(appendStringValues(under_test, {"CHILD2", "CHILD3", "NOT_IN_TREE"}).has_value());

   auto tree_node_id_child = metadata.phylo_tree->getTreeNodeId("CHILD");
   auto tree_node_id_child2 = metadata.phylo_tree->getTreeNodeId("CHILD2");
   auto tree_node_id_child3 = metadata.phylo_tree->getTreeNodeId("CHILD3");
   auto tree_node_id_not_in_tree = metadata.phylo_tree->getTreeNodeId("NOT_IN_TREE");
   auto tree_node_id_root = metadata.phylo_tree->getTreeNodeId("ROOT");

   EXPECT_EQ(under_test.getValueString(0), "CHILD2");
   EXPECT_EQ(under_test.getValueString(1), "CHILD3");
   EXPECT_EQ(under_test.getValueString(2), "NOT_IN_TREE");
   EXPECT_EQ(under_test.getDescendants(tree_node_id_child2.value()).cardinality(), 0);
   EXPECT_EQ(under_test.getDescendants(tree_node_id_child.value()).cardinality(), 2);
   EXPECT_EQ(under_test.getDescendants(tree_node_id_root.value()).cardinality(), 2);

   EXPECT_EQ(tree_node_id_not_in_tree, std::nullopt);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(StringColumn, duplicatePhyloLeafLeavesColumnUnmodified) {
   auto phylo_tree = silo::common::PhyloTree::fromNewickString(
      "((CHILD2:0.5, CHILD3:1)CHILD:0.1, NOT_IN_DATASET:1.5)ROOT;"
   );
   StringColumnMetadata metadata{"string_column", std::move(phylo_tree)};
   StringColumn under_test(&metadata);

   // First append a valid leaf so we have committed state that must survive the failed append.
   SILO_ASSERT(appendStringValues(under_test, {"CHILD2"}).has_value());

   // A buffer that re-uses CHILD2 (already committed) with a null in between must fail, and must
   // not leave the null in null_bitmap or grow the column.
   StringColumn::Builder builder;
   builder.insert("CHILD3");
   builder.insertNull();
   builder.insert("CHILD2");
   auto result = under_test.appendChunk(builder.finalize());

   ASSERT_FALSE(result.has_value());
   EXPECT_EQ(result.error(), "Node 'CHILD2' already exists in the phylogenetic tree.");
   EXPECT_EQ(under_test.numValues(), 1);
   EXPECT_TRUE(under_test.null_bitmap.isEmpty());
   // CHILD3 must not have been bound, since the whole buffer was rejected.
   EXPECT_EQ(
      metadata.phylo_tree->nodes.at(silo::common::TreeNodeId{"CHILD3"})->row_index, std::nullopt
   );
   EXPECT_EQ(metadata.phylo_tree->nodes.at(silo::common::TreeNodeId{"CHILD2"})->row_index, 0);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(StringColumn, duplicatePhyloLeafWithinSingleBufferIsRejected) {
   auto phylo_tree = silo::common::PhyloTree::fromNewickString(
      "((CHILD2:0.5, CHILD3:1)CHILD:0.1, NOT_IN_DATASET:1.5)ROOT;"
   );
   StringColumnMetadata metadata{"string_column", std::move(phylo_tree)};
   StringColumn under_test(&metadata);

   // The same leaf appears twice within one buffer; rowIndexExists() cannot catch this, so the
   // in-buffer claim tracking must.
   auto result = appendStringValues(under_test, {"CHILD2", "CHILD2"});

   ASSERT_FALSE(result.has_value());
   EXPECT_EQ(under_test.numValues(), 0);
   EXPECT_EQ(
      metadata.phylo_tree->nodes.at(silo::common::TreeNodeId{"CHILD2"})->row_index, std::nullopt
   );
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(StringColumn, rawInsertedValuesRequeryLongValue) {
   StringColumnMetadata column("string_column");
   StringColumn under_test{&column};

   SILO_ASSERT(appendStringValues(
                  under_test,
                  {"value 1",
                   "value 2",
                   "value 2",
                   "value 3",
                   "some string that is a little longer 1",
                   "value 1"}
   )
                  .has_value());

   EXPECT_EQ(under_test.getValueString(4), "some string that is a little longer 1");
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(StringColumn, compareAcrossColumns) {
   StringColumnMetadata under_test("string_column");
   StringColumn column_1{&under_test};
   SILO_ASSERT(appendStringValues(
                  column_1,
                  {"value 1",
                   "value 2",
                   "value 2",
                   "value 3",
                   "some string that is a little longer 1",
                   "value 1"}
   )
                  .has_value());

   StringColumn column_2{&under_test};
   SILO_ASSERT(appendStringValues(
                  column_2,
                  {"other value 2",
                   "other values 3",
                   "value 1",
                   "other value 3",
                   "some string that is a little longer 1",
                   "other value 1"}
   )
                  .has_value());

   EXPECT_EQ(column_1.getValueString(0), column_1.getValueString(5));
   EXPECT_EQ(column_1.getValueString(5), column_2.getValueString(2));
   EXPECT_EQ(column_1.getValueString(4), column_2.getValueString(4));
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(StringColumn, valuesSpanningMultipleAppendedChunks) {
   StringColumnMetadata metadata{"string_column"};
   StringColumn under_test(&metadata);

   // Each appendChunk starts a fresh, immutable chunk. Row ids continue across chunk boundaries and
   // both short (in-place) and long (suffix in variable data) values must resolve to their own
   // chunk's registries.
   SILO_ASSERT(
      appendStringValues(under_test, {"short a", "a long value that spills over 1"}).has_value()
   );
   SILO_ASSERT(
      appendStringValues(under_test, {"short b", "a long value that spills over 2", "short c"})
         .has_value()
   );
   SILO_ASSERT(appendStringValues(under_test, {"a long value that spills over 3"}).has_value());

   EXPECT_EQ(under_test.numValues(), 6);
   EXPECT_EQ(under_test.getValueString(0), "short a");
   EXPECT_EQ(under_test.getValueString(1), "a long value that spills over 1");
   EXPECT_EQ(under_test.getValueString(2), "short b");
   EXPECT_EQ(under_test.getValueString(3), "a long value that spills over 2");
   EXPECT_EQ(under_test.getValueString(4), "short c");
   EXPECT_EQ(under_test.getValueString(5), "a long value that spills over 3");
}

TEST(StringColumn, manyLongValues) {
   std::vector<std::string> test_values;
   test_values.reserve(50000);
   for (size_t i = 0; i < 50000; ++i) {
      test_values.push_back(fmt::format("SOME_{}_LONG_{}_STRING", i, i));
   }

   StringColumnMetadata under_test("string_column");
   StringColumn column{&under_test};

   StringColumn::Builder builder;
   for (auto& value : test_values) {
      builder.insert(value);
   }
   SILO_ASSERT(column.appendChunk(builder.finalize()).has_value());

   for (size_t i = 0; i < 50000; ++i) {
      ASSERT_EQ(column.getValue(i).fastCompare(test_values.at(i)), std::nullopt);
      ASSERT_EQ(column.getValueString(i), test_values.at(i));
   }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(StringColumn, manyMixedValues) {
   std::vector<std::string> test_values;
   test_values.reserve(50001);
   for (size_t i = 0; i < 50001; ++i) {
      if (i % 2 == 1) {
         test_values.emplace_back("SHRT");
      } else if (i % 10000 == 0) {
         test_values.push_back(fmt::format("{}_VERY_VERY_LONG_STRING_{}", i, std::string(i, 'x')));
      } else {
         test_values.push_back(fmt::format("{}_LONG_STRING_{}", i, std::string(100, 'x')));
      }
   }

   StringColumnMetadata under_test("string_column");
   StringColumn column{&under_test};

   StringColumn::Builder builder;
   for (auto& value : test_values) {
      builder.insert(value);
   }
   SILO_ASSERT(column.appendChunk(builder.finalize()).has_value());

   for (size_t i = 0; i < 50001; ++i) {
      if (i % 2 == 1) {
         ASSERT_TRUE(column.getValue(i).fastCompare(test_values.at(i)).has_value());
         ASSERT_EQ(
            column.getValue(i).fastCompare(test_values.at(i)).value(), std::strong_ordering::equal
         );
         ASSERT_EQ(column.getValueString(i), test_values.at(i));
      } else {
         ASSERT_EQ(column.getValue(i).fastCompare(test_values.at(i)), std::nullopt);
         ASSERT_EQ(column.getValueString(i), test_values.at(i));
      }
   }
}
