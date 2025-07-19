#include "silo/common/phylo_tree.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <yaml-cpp/exceptions.h>

#include "silo/preprocessing/preprocessing_exception.h"

using silo::common::PhyloTree;
using silo::common::TreeNodeId;

TEST(PhyloTree, correctlyParsesFromJSON) {
   auto phylo_tree_file = PhyloTree::fromAuspiceJSONString(
      R"({  
  "version": "schema version",  
  "meta": {},  
  "tree": {  
    "name": "ROOT",  
    "children": [  
      {  
        "name": "CHILD",  
        "children": [  
          {  
            "name": "CHILD2"  
          }  
        ]  
      }  
    ]  
  }  
})"
   );
   ASSERT_EQ(phylo_tree_file.nodes.size(), 3);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->parent, std::nullopt);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->depth, 0);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->children.size(), 1);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->depth, 1);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->children.size(), 1);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->children.at(0), TreeNodeId{"CHILD2"});
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD2"})->parent, TreeNodeId{"CHILD"});
}

TEST(PhyloTree, throwsOnInvalidJSON) {
   EXPECT_THROW(
      PhyloTree::fromAuspiceJSONString("{\"invalid\": \"json\"}"),
      silo::preprocessing::PreprocessingException
   );
}

TEST(PhyloTree, throwsOnInvalidAuspiceJSONDuplicateNodeId) {
   EXPECT_THROW(
      PhyloTree::fromAuspiceJSONString(R"({  
  "version": "schema version",  
  "meta": {},  
  "tree": {  
    "name": "ROOT",  
    "children": [  
      {  
        "name": "CHILD",  
        "children": [  
          {  
            "name": "CHILD"  
          }  
        ]  
      }  
    ]  
  }  
})"),
      silo::preprocessing::PreprocessingException
   );
}

TEST(PhyloTree, correctlyParsesFromNewick) {
   auto phylo_tree_file = PhyloTree::fromNewickString("((CHILD2)CHILD)ROOT;");
   ASSERT_EQ(phylo_tree_file.nodes.size(), 3);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->parent, std::nullopt);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->depth, 0);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->children.size(), 1);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->depth, 1);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->children.size(), 1);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->children.at(0), TreeNodeId{"CHILD2"});
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD2"})->parent, TreeNodeId{"CHILD"});
}

TEST(PhyloTree, correctlyParsesFromNewickWithNewLine) {
   auto phylo_tree_file = PhyloTree::fromNewickString("(CHILD)ROOT;\n");
   ASSERT_EQ(phylo_tree_file.nodes.size(), 2);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->parent, std::nullopt);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->depth, 0);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->children.size(), 1);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->depth, 1);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->children.size(), 0);
}

TEST(PhyloTree, correctlyParsesFromNewickWithBranchLengths) {
   auto phylo_tree_file =
      PhyloTree::fromNewickString("((CHILD2:0.5, CHILD3:1)CHILD:0.1, CHILD4:1.5)ROOT;");
   ASSERT_EQ(phylo_tree_file.nodes.size(), 5);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->parent, std::nullopt);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->depth, 0);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->children.size(), 2);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->depth, 1);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->children.size(), 2);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->children.at(1), TreeNodeId{"CHILD2"});
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD2"})->parent, TreeNodeId{"CHILD"});
}

TEST(PhyloTree, throwsOnInvalidNewick) {
   EXPECT_THROW(
      PhyloTree::fromNewickString("((CHILD2)CHILD;"), silo::preprocessing::PreprocessingException
   );
}

TEST(PhyloTree, throwsOnNewickWithInvalidCharacters) {
   try {
      PhyloTree::fromNewickString("(CHILD%)CHILD;");
      FAIL() << "Expected PreprocessingException";
   } catch (const silo::preprocessing::PreprocessingException& e) {
      EXPECT_THAT(
         std::string(e.what()),
         ::testing::HasSubstr("Newick string contains invalid characters: '%'")
      );
   } catch (...) {
      FAIL() << "Expected PreprocessingException, but caught a different exception";
   }
}

TEST(PhyloTree, throwsOnInvalidNewickNoSemicolon) {
   EXPECT_THROW(
      PhyloTree::fromNewickString("((CHILD2)CHILD)ROOT"),
      silo::preprocessing::PreprocessingException
   );
}

TEST(PhyloTree, throwsOnInvalidNewickWithDuplicateNodeId) {
   EXPECT_THROW(
      PhyloTree::fromNewickString("((CHILD)CHILD)ROOT"), silo::preprocessing::PreprocessingException
   );
}

TEST(PhyloTree, correctlyReturnsMRCA) {
   auto phylo_tree_file = PhyloTree::fromNewickString("((CHILD2, CHILD3)CHILD, CHILD4)ROOT;");
   auto mrca_response = phylo_tree_file.getMRCA({"CHILD2", "CHILD3"});
   ASSERT_TRUE(mrca_response.mrca_node_id.has_value());
   ASSERT_EQ(mrca_response.mrca_node_id.value(), TreeNodeId{"CHILD"});
   ASSERT_TRUE(mrca_response.not_in_tree.empty());

   mrca_response = phylo_tree_file.getMRCA({"CHILD2", "NOT_IN_TREE"});
   ASSERT_TRUE(mrca_response.mrca_node_id.has_value());
   ASSERT_EQ(mrca_response.mrca_node_id.value(), TreeNodeId{"CHILD2"});
   ASSERT_TRUE(
      mrca_response.not_in_tree.size() == 1 && mrca_response.not_in_tree[0] == "NOT_IN_TREE"
   );

   mrca_response = phylo_tree_file.getMRCA({"CHILD2", "CHILD3", "CHILD4"});
   ASSERT_TRUE(mrca_response.mrca_node_id.has_value());
   ASSERT_EQ(mrca_response.mrca_node_id.value(), TreeNodeId{"ROOT"});
   ASSERT_TRUE(mrca_response.not_in_tree.empty());

   mrca_response = phylo_tree_file.getMRCA({"NOT_IN_TREE", "NOT_IN_TREE2"});
   ASSERT_FALSE(mrca_response.mrca_node_id.has_value());
   ASSERT_TRUE(
      mrca_response.not_in_tree.size() == 2 && mrca_response.not_in_tree[0] == "NOT_IN_TREE" &&
      mrca_response.not_in_tree[1] == "NOT_IN_TREE2"
   );
}
