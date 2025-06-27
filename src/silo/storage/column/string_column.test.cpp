#include "silo/storage/column/string_column.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>

#include <gtest/gtest.h>
#include "silo/common/phylo_tree.h"
#include "silo/common/tree_node_id.h"

using silo::storage::column::StringColumnMetadata;
using silo::storage::column::StringColumnPartition;

TEST(StringColumnPartition, rawInsertedValuesRequeried) {
   StringColumnMetadata metadata{"string_column"};
   StringColumnPartition under_test(&metadata);

   under_test.insert("value 1");
   under_test.insert("value 2");
   under_test.insert("value 2");
   under_test.insert("value 3");
   under_test.insert("some string that is a little longer 1");
   under_test.insert("value 1");

   EXPECT_EQ(under_test.getValues()[0].toString(metadata.dictionary), "value 1");
   EXPECT_EQ(under_test.getValues()[1].toString(metadata.dictionary), "value 2");
   EXPECT_EQ(under_test.getValues()[2].toString(metadata.dictionary), "value 2");
   EXPECT_EQ(under_test.getValues()[3].toString(metadata.dictionary), "value 3");
   EXPECT_EQ(
      under_test.getValues()[4].toString(metadata.dictionary),
      "some string that is a little longer 1"
   );
   EXPECT_EQ(under_test.getValues()[5].toString(metadata.dictionary), "value 1");
}

TEST(StringColumnPartition, serializationOfMetadataWorks) {
   auto phylo_tree = silo::common::PhyloTree::fromNewickString(
      "((CHILD2:0.5, CHILD3:1)CHILD:0.1, NOT_IN_DATASET:1.5)ROOT;"
   );
   StringColumnMetadata metadata{"string_column", std::move(phylo_tree)};
   StringColumnPartition partition(&metadata);

   partition.insert("CHILD2");
   partition.insert("CHILD3");
   partition.insert("NOT_IN_TREE");

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

TEST(StringColumnPartition, rawInsertedValuesWithPhyloTreeRequeried) {
   auto phylo_tree = silo::common::PhyloTree::fromNewickString(
      "((CHILD2:0.5, CHILD3:1)CHILD:0.1, NOT_IN_DATASET:1.5)ROOT;"
   );
   StringColumnMetadata metadata{"string_column", std::move(phylo_tree)};
   StringColumnPartition under_test(&metadata);

   under_test.insert("CHILD2");
   under_test.insert("CHILD3");
   under_test.insert("NOT_IN_TREE");

   auto tree_node_id_child = metadata.phylo_tree->getTreeNodeId("CHILD");
   auto tree_node_id_child2 = metadata.phylo_tree->getTreeNodeId("CHILD2");
   auto tree_node_id_child3 = metadata.phylo_tree->getTreeNodeId("CHILD3");
   auto tree_node_id_not_in_tree = metadata.phylo_tree->getTreeNodeId("NOT_IN_TREE");
   auto tree_node_id_root = metadata.phylo_tree->getTreeNodeId("ROOT");

   EXPECT_EQ(under_test.getValues()[0].toString(metadata.dictionary), "CHILD2");
   EXPECT_EQ(under_test.getValues()[1].toString(metadata.dictionary), "CHILD3");
   EXPECT_EQ(under_test.getValues()[2].toString(metadata.dictionary), "NOT_IN_TREE");
   EXPECT_EQ(under_test.getDescendants(tree_node_id_child2.value()).cardinality(), 0);
   EXPECT_EQ(under_test.getDescendants(tree_node_id_child.value()).cardinality(), 2);
   EXPECT_EQ(under_test.getDescendants(tree_node_id_root.value()).cardinality(), 2);

   EXPECT_EQ(tree_node_id_not_in_tree, std::nullopt);
}

TEST(StringColumn, rawInsertedValuesRequeried) {
   StringColumnMetadata column("string_column");
   StringColumnPartition under_test{&column};

   under_test.insert("value 1");
   under_test.insert("value 2");
   under_test.insert("value 2");
   under_test.insert("value 3");
   under_test.insert("some string that is a little longer 1");
   under_test.insert("value 1");

   const silo::common::String somehow_acquired_element_representation = under_test.getValues()[4];

   EXPECT_EQ(
      under_test.lookupValue(somehow_acquired_element_representation),
      "some string that is a little longer 1"
   );
}

TEST(StringColumn, compareAcrossPartitions) {
   StringColumnMetadata under_test("string_column");
   StringColumnPartition partition_1{&under_test};
   partition_1.insert("value 1");
   partition_1.insert("value 2");
   partition_1.insert("value 2");
   partition_1.insert("value 3");
   partition_1.insert("some string that is a little longer 1");
   partition_1.insert("value 1");

   StringColumnPartition partition_2{&under_test};
   partition_2.insert("other value 2");
   partition_2.insert("other values 3");
   partition_2.insert("value 1");
   partition_2.insert("other value 3");
   partition_2.insert("some string that is a little longer 1");
   partition_2.insert("other value 1");

   EXPECT_EQ(partition_1.getValues()[0], partition_1.getValues()[5]);
   EXPECT_EQ(partition_1.getValues()[5], partition_2.getValues()[2]);
   EXPECT_EQ(partition_1.getValues()[4], partition_2.getValues()[4]);
}
