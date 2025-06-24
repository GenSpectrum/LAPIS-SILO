#include "silo/storage/column/string_column.h"

#include <gtest/gtest.h>
#include "silo/common/phylo_tree.h"
#include "silo/query_engine/bad_request.h"

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

TEST(StringColumnPartition, rawInsertedValuesWithPhyloTreeRequeried) {
   auto phylo_tree = silo::common::PhyloTree::fromNewickString(
      "((CHILD2:0.5, CHILD3:1)CHILD:0.1, NOT_IN_DATASET:1.5)ROOT;"
   );
   StringColumnMetadata metadata{"string_column", std::move(phylo_tree)};
   StringColumnPartition under_test(&metadata);

   under_test.insert("CHILD2");
   under_test.insert("CHILD3");
   under_test.insert("NOT_IN_TREE");

   EXPECT_EQ(under_test.getValues()[0].toString(metadata.dictionary), "CHILD2");
   EXPECT_EQ(under_test.getValues()[1].toString(metadata.dictionary), "CHILD3");
   EXPECT_EQ(under_test.getValues()[2].toString(metadata.dictionary), "NOT_IN_TREE");
   EXPECT_EQ(under_test.getDescendants("CHILD2").cardinality(), 0);
   EXPECT_EQ(under_test.getDescendants("CHILD").cardinality(), 2);
   EXPECT_EQ(under_test.getDescendants("ROOT").cardinality(), 2);

   ASSERT_THROW(under_test.getDescendants("NOT_IN_TREE"), silo::BadRequest);
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
