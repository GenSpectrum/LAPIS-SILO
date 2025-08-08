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
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->branch_length, std::nullopt);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->children.at(0), TreeNodeId{"CHILD2"});
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD2"})->parent, TreeNodeId{"CHILD"});
}

TEST(PhyloTree, correctlyParsesFromJSONwithBranchLengths) {
   auto phylo_tree_file = PhyloTree::fromAuspiceJSONString(
      R"({  
  "version": "schema version",  
  "meta": {},  
  "tree": {  
    "name": "ROOT",  
    "children": [  
      {  
        "name": "CHILD",  
         "node_attrs": {  
            "div": 0.1  
         },
        "children": [  
          {  
            "name": "CHILD2",
            "node_attrs": {  
              "div": 0.5  
            } 
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
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->branch_length, 0.1f);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->children.at(0), TreeNodeId{"CHILD2"});
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD2"})->parent, TreeNodeId{"CHILD"});
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD2"})->branch_length, 0.5f);
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
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->branch_length, std::nullopt);
}

TEST(PhyloTree, correctlyParsesFromNewickWithBranchLengths) {
   auto phylo_tree_file =
      PhyloTree::fromNewickString("((CHILD2:0.5, CHILD3:1)CHILD:0.1, CHILD4:1.5)ROOT;");
   ASSERT_EQ(phylo_tree_file.nodes.size(), 5);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->parent, std::nullopt);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->branch_length, std::nullopt);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->depth, 0);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"ROOT"})->children.size(), 2);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->depth, 1);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD"})->branch_length, 0.1f);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD2"})->branch_length, 0.5f);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD3"})->branch_length, 1.0f);
   ASSERT_EQ(phylo_tree_file.nodes.at(TreeNodeId{"CHILD4"})->branch_length, 1.5f);
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

   std::vector<std::string> expected = {"NOT_IN_TREE", "NOT_IN_TREE2"};
   mrca_response = phylo_tree_file.getMRCA({"NOT_IN_TREE", "NOT_IN_TREE2"});
   ASSERT_FALSE(mrca_response.mrca_node_id.has_value());
   ASSERT_TRUE(mrca_response.not_in_tree.size() == 2 && mrca_response.not_in_tree == expected);
}

TEST(PhyloTree, correctlyReturnsParents) {
   auto phylo_tree_file = PhyloTree::fromNewickString("((CHILD2, CHILD3)CHILD, CHILD4)ROOT;");
   auto parents_response = phylo_tree_file.getParents({"CHILD2", "CHILD3"});
   ASSERT_TRUE(parents_response.parent_node_ids.size() == 1);
   std::unordered_set<std::optional<TreeNodeId>> expected_parents = {TreeNodeId{"CHILD"}};
   ASSERT_EQ(parents_response.parent_node_ids, expected_parents);
   ASSERT_TRUE(parents_response.not_in_tree.empty());

   parents_response = phylo_tree_file.getParents({"CHILD2", "NOT_IN_TREE"});
   ASSERT_TRUE(parents_response.parent_node_ids.size() == 1);
   ASSERT_EQ(parents_response.parent_node_ids, expected_parents);
   ASSERT_TRUE(
      parents_response.not_in_tree.size() == 1 && parents_response.not_in_tree[0] == "NOT_IN_TREE"
   );

   parents_response = phylo_tree_file.getParents({"CHILD2", "CHILD3", "CHILD4"});
   ASSERT_TRUE(parents_response.parent_node_ids.size() == 2);
   expected_parents = {TreeNodeId{"ROOT"}, TreeNodeId{"CHILD"}};
   ASSERT_EQ(parents_response.parent_node_ids, expected_parents);
   ASSERT_TRUE(parents_response.not_in_tree.empty());

   std::vector<std::string> expected = {"NOT_IN_TREE", "NOT_IN_TREE2"};
   parents_response = phylo_tree_file.getParents({"NOT_IN_TREE", "NOT_IN_TREE2"});
   ASSERT_TRUE(parents_response.parent_node_ids.size() == 0);
   ASSERT_TRUE(
      parents_response.not_in_tree.size() == 2 && parents_response.not_in_tree == expected
   );
}

TEST(PhyloTree, correctlyReturnsSubTreeNewick) {
   auto phylo_tree =
      PhyloTree::fromNewickString("(((A1.1, A1.2)A1,(A2.1)A2)A,(B1,(B2.1,B2.2)B2)B)R;");
   auto subtree_left_side =
      phylo_tree.toNewickString({"A1.1", "A1.2", "A2.1"}, false).newick_string;
   ASSERT_EQ(subtree_left_side, "((A1.1,A1.2)A1,(A2.1)A2)A;");
   auto subtree_right_side = phylo_tree.toNewickString({"B1", "B2.1", "B2.2"}, false).newick_string;
   ASSERT_EQ(subtree_right_side, "(B1,(B2.1,B2.2)B2)B;");
   auto subtree_full =
      phylo_tree.toNewickString({"A1.1", "A1.2", "A2.1", "B1", "B2.1", "B2.2"}, false)
         .newick_string;
   ASSERT_EQ(subtree_full, "(((A1.1,A1.2)A1,(A2.1)A2)A,(B1,(B2.1,B2.2)B2)B)R;");
   auto subtree_empty = phylo_tree.toNewickString({"NOT_IN_TREE"}, false).newick_string;
   ASSERT_EQ(subtree_empty, "");
   auto subtree_one_node = phylo_tree.toNewickString({"A1.1"}, false).newick_string;
   ASSERT_EQ(subtree_one_node, "A1.1;");
}

TEST(PhyloTree, correctlyReturnsSubTreeNewickWithBranchLengths) {
   auto phylo_tree = PhyloTree::fromNewickString(
      "(((A1.1:0.2, "
      "A1.2:0.2)A1:0.3,(A2.1:0)A2:0.4)A:0.2,(B1:0.5,(B2.1:0.3,B2.2:0.05)B2:0.05)B:0.5)R;"
   );
   auto subtree_left_side =
      phylo_tree.toNewickString({"A1.1", "A1.2", "A2.1"}, false).newick_string;
   ASSERT_EQ(subtree_left_side, "((A1.1:0.2,A1.2:0.2)A1:0.3,(A2.1:0)A2:0.4)A;");
   auto subtree_right_side = phylo_tree.toNewickString({"B1", "B2.1"}, false).newick_string;
   ASSERT_EQ(subtree_right_side, "(B1:0.5,(B2.1:0.3)B2:0.05)B;");
   auto subtree_full =
      phylo_tree.toNewickString({"A1.1", "A1.2", "A2.1", "B1", "B2.1", "B2.2"}, false)
         .newick_string;
   ASSERT_EQ(
      subtree_full,
      "(((A1.1:0.2,A1.2:0.2)A1:0.3,(A2.1:0)A2:0.4)A:0.2,(B1:0.5,(B2.1:0.3,B2.2:0.05)B2:0.05)B:0.5)"
      "R;"
   );
   auto subtree_empty = phylo_tree.toNewickString({"NOT_IN_TREE"}, false).newick_string;
   ASSERT_EQ(subtree_empty, "");
   auto subtree_one_node = phylo_tree.toNewickString({"A1.1"}, false).newick_string;
   ASSERT_EQ(subtree_one_node, "A1.1;");
}

TEST(PhyloTree, correctlyReturnsSubTreeNewickWithContractUnaryNodes) {
   auto phylo_tree =
      PhyloTree::fromNewickString("(((A1.1, A1.2)A1,(A2.1)A2)A,(B1,(B2.1,B2.2)B2)B)R;");
   auto subtree_left_side = phylo_tree.toNewickString({"A1.1", "A1.2", "A2.1"}, true).newick_string;
   ASSERT_EQ(subtree_left_side, "((A1.1,A1.2)A1,A2.1)A;");
   auto subtree_right_side = phylo_tree.toNewickString({"B1", "B2.1", "B2.2"}, true).newick_string;
   ASSERT_EQ(subtree_right_side, "(B1,(B2.1,B2.2)B2)B;");
   auto subtree_full =
      phylo_tree.toNewickString({"A1.1", "A1.2", "A2.1", "B1", "B2.1", "B2.2"}, true).newick_string;
   ASSERT_EQ(subtree_full, "(((A1.1,A1.2)A1,A2.1)A,(B1,(B2.1,B2.2)B2)B)R;");
   auto subtree_empty = phylo_tree.toNewickString({"NOT_IN_TREE"}, true).newick_string;
   ASSERT_EQ(subtree_empty, "");
   auto subtree_one_node = phylo_tree.toNewickString({"A1.1"}, true).newick_string;
   ASSERT_EQ(subtree_one_node, "A1.1;");
}

TEST(PhyloTree, correctlyReturnsSubTreeNewickWithContractUnaryNodesWithBranchLengths) {
   auto phylo_tree = PhyloTree::fromNewickString(
      "(((A1.1:0.2, "
      "A1.2:0.2)A1:0.3,(A2.1:0)A2:0.4)A:0.2,(B1:0.5,(B2.1:0.3,B2.2:0.05)B2:0.05)B:0.5)R;"
   );
   auto subtree_left_side =

      phylo_tree.toNewickString({"A1.1", "A1.2", "A2.1"}, true).newick_string;
   ASSERT_EQ(subtree_left_side, "((A1.1:0.2,A1.2:0.2)A1:0.3,A2.1:0.4)A;");
   auto subtree_right_side = phylo_tree.toNewickString({"B1", "B2.1"}, true).newick_string;
   ASSERT_EQ(subtree_right_side, "(B1:0.5,B2.1:0.35)B;");
   auto subtree_full =
      phylo_tree.toNewickString({"A1.1", "A1.2", "A2.1", "B1", "B2.1", "B2.2"}, true).newick_string;
   ASSERT_EQ(
      subtree_full,
      "(((A1.1:0.2,A1.2:0.2)A1:0.3,A2.1:0.4)A:0.2,(B1:0.5,(B2.1:0.3,B2.2:0.05)B2:0.05)B:0.5)R;"
   );
   auto subtree_empty = phylo_tree.toNewickString({"NOT_IN_TREE"}, true).newick_string;
   ASSERT_EQ(subtree_empty, "");
   auto subtree_one_node = phylo_tree.toNewickString({"A1.1"}, true).newick_string;
   ASSERT_EQ(subtree_one_node, "A1.1;");
}
