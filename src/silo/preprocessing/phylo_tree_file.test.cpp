#include "silo/preprocessing/phylo_tree_file.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <yaml-cpp/exceptions.h>

#include "silo/preprocessing/preprocessing_exception.h"

using silo::preprocessing::PhyloTreeFile;

TEST(PhyloTreeFile, correctlyParsesFromJSON) {
   auto phylo_tree_file = PhyloTreeFile::fromAuspiceJSONString(
      "{"
      "\"version\": \"schema version\","
      "\"meta\": {},"
      "\"tree\": {"
      "\"name\": \"ROOT\","
      "\"children\": ["
      "{"
      "\"name\": \"CHILD\","
      "\"children\": ["
      "{"
      "\"name\": \"CHILD2\""
      "}"
      "]"
      "}"
      "]"
      "}"
      "}"
   );
   ASSERT_EQ(phylo_tree_file.nodes.size(), 3);
   ASSERT_EQ(phylo_tree_file.nodes.at("ROOT")->parent, std::nullopt);
   ASSERT_EQ(phylo_tree_file.nodes.at("ROOT")->depth, 0);
   ASSERT_EQ(phylo_tree_file.nodes.at("ROOT")->children.size(), 1);
   ASSERT_EQ(phylo_tree_file.nodes.at("CHILD")->depth, 1);
   ASSERT_EQ(phylo_tree_file.nodes.at("CHILD")->children.size(), 1);
   ASSERT_EQ(phylo_tree_file.nodes.at("CHILD")->children.at(0)->node_id, "CHILD2");
   ASSERT_EQ(phylo_tree_file.nodes.at("CHILD2")->parent->get()->node_id, "CHILD");
}

TEST(PhyloTreeFile, throwsOnInvalidJSON) {
   EXPECT_THROW(
      PhyloTreeFile::fromAuspiceJSONString("{\"invalid\": \"json\"}"),
      silo::preprocessing::PreprocessingException
   );
}

TEST(PhyloTreeFile, correctlyParsesFromNewick) {
   auto phylo_tree_file = PhyloTreeFile::fromNewickString("((CHILD2)CHILD)ROOT;");
   ASSERT_EQ(phylo_tree_file.nodes.size(), 3);
   ASSERT_EQ(phylo_tree_file.nodes.at("ROOT")->parent, std::nullopt);
   ASSERT_EQ(phylo_tree_file.nodes.at("ROOT")->depth, 0);
   ASSERT_EQ(phylo_tree_file.nodes.at("ROOT")->children.size(), 1);
   ASSERT_EQ(phylo_tree_file.nodes.at("CHILD")->depth, 1);
   ASSERT_EQ(phylo_tree_file.nodes.at("CHILD")->children.size(), 1);
   ASSERT_EQ(phylo_tree_file.nodes.at("CHILD")->children.at(0)->node_id, "CHILD2");
   ASSERT_EQ(phylo_tree_file.nodes.at("CHILD2")->parent->get()->node_id, "CHILD");
}

TEST(PhyloTreeFile, correctlyParsesFromNewickWithBranchLengths) {
   auto phylo_tree_file =
      PhyloTreeFile::fromNewickString("((CHILD2:0.5, CHILD3:1)CHILD:0.1, CHILD4:1.5)ROOT;");
   ASSERT_EQ(phylo_tree_file.nodes.size(), 5);
   ASSERT_EQ(phylo_tree_file.nodes.at("ROOT")->parent, std::nullopt);
   ASSERT_EQ(phylo_tree_file.nodes.at("ROOT")->depth, 0);
   ASSERT_EQ(phylo_tree_file.nodes.at("ROOT")->children.size(), 2);
   ASSERT_EQ(phylo_tree_file.nodes.at("CHILD")->depth, 1);
   ASSERT_EQ(phylo_tree_file.nodes.at("CHILD")->children.size(), 2);
   ASSERT_EQ(phylo_tree_file.nodes.at("CHILD")->children.at(0)->node_id, "CHILD2");
   ASSERT_EQ(phylo_tree_file.nodes.at("CHILD2")->parent->get()->node_id, "CHILD");
}

TEST(PhyloTreeFile, throwsOnInvalidNewick) {
   EXPECT_THROW(
      PhyloTreeFile::fromNewickString("((CHILD2)CHILD;"),
      silo::preprocessing::PreprocessingException
   );
}

TEST(PhyloTreeFile, throwsOnInvalidNewickNoSemicolon) {
   EXPECT_THROW(
      PhyloTreeFile::fromNewickString("((CHILD2)CHILD)ROOT"),
      silo::preprocessing::PreprocessingException
   );
}
