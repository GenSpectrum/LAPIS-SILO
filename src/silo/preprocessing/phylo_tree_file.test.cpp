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
      "\"name\": \"CHILD2\","
      "\"children\": []"
      "}"
      "]"
      "}"
      "]"
      "}"
      "}"
   );
   ASSERT_EQ(phylo_tree_file.nodes.size(), 3);
   ASSERT_EQ(phylo_tree_file.nodes.at("ROOT").parent, std::nullopt);
   ASSERT_EQ(phylo_tree_file.nodes.at("ROOT").depth, 0);
   ASSERT_EQ(phylo_tree_file.nodes.at("ROOT").children.size(), 1);
   ASSERT_EQ(phylo_tree_file.nodes.at("CHILD").depth, 1);
   ASSERT_EQ(phylo_tree_file.nodes.at("CHILD").children.size(), 1);
   ASSERT_EQ(phylo_tree_file.nodes.at("CHILD").children.at(0), "CHILD2");
}