#include "silo/common/lineage_tree.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/preprocessing/lineage_definition_file.h"
#include "silo/preprocessing/preprocessing_exception.h"

using silo::common::LineageTreeAndIdMap;
using silo::preprocessing::LineageDefinitionFile;

TEST(LineageTreeAndIdMap, correctSimpleTree) {
   auto lineage_definition_file = LineageDefinitionFile::fromYAML(R"(
BASE:
  parents: []
CHILD:
  parents:
    - BASE
)");
   auto lineage_tree = LineageTreeAndIdMap::fromLineageDefinitionFile(lineage_definition_file);
   ASSERT_TRUE(lineage_tree.lineage_id_lookup_map.getId("BASE").has_value());
   ASSERT_TRUE(lineage_tree.lineage_id_lookup_map.getId("CHILD").has_value());
   ASSERT_FALSE(lineage_tree.lineage_id_lookup_map.getId("Base").has_value());
   ASSERT_FALSE(lineage_tree.lineage_id_lookup_map.getId("base").has_value());

   ASSERT_TRUE(lineage_tree.lineage_tree
                  .getParent(lineage_tree.lineage_id_lookup_map.getId("CHILD").value())
                  .has_value());
   ASSERT_EQ(
      lineage_tree.lineage_tree.getParent(lineage_tree.lineage_id_lookup_map.getId("CHILD").value()
      ),
      lineage_tree.lineage_id_lookup_map.getId("BASE")
   );
}

TEST(LineageTreeAndIdMap, errorOnMissingParent) {
   EXPECT_THAT(
      []() {
         (void)LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAML(R"(
some_lineage:
  parents:
    - parent_that_does_not_exist
)"));
      },
      ThrowsMessage<silo::preprocessing::PreprocessingException>(::testing::HasSubstr(
         "The lineage 'parent_that_does_not_exist' which is specified as the parent of "
         "vertex 'some_lineage' does not have a definition itself."
      ))
   );
}

TEST(LineageTreeAndIdMap, correctTreeRelations) {
   auto lineage_definition_file = LineageDefinitionFile::fromYAML(R"(
BASE:
  aliases: [ base_alias ]
  parents: []
CHILD1:
  parents:
    - BASE
CHILD2:
  parents:
    - base_alias
GRANDCHILD1:
  parents:
    - CHILD1
GRANDCHILD2:
  parents:
    - CHILD1
)");
   auto lineage_tree = LineageTreeAndIdMap::fromLineageDefinitionFile(lineage_definition_file);
   ASSERT_TRUE(lineage_tree.lineage_id_lookup_map.getId("BASE").has_value());
   ASSERT_TRUE(lineage_tree.lineage_id_lookup_map.getId("CHILD1").has_value());
   ASSERT_TRUE(lineage_tree.lineage_id_lookup_map.getId("CHILD2").has_value());
   ASSERT_TRUE(lineage_tree.lineage_id_lookup_map.getId("GRANDCHILD1").has_value());
   ASSERT_TRUE(lineage_tree.lineage_id_lookup_map.getId("GRANDCHILD2").has_value());
   auto base = lineage_tree.lineage_id_lookup_map.getId("BASE").value();
   auto child1 = lineage_tree.lineage_id_lookup_map.getId("CHILD1").value();
   auto child2 = lineage_tree.lineage_id_lookup_map.getId("CHILD2").value();
   auto grandchild1 = lineage_tree.lineage_id_lookup_map.getId("GRANDCHILD1").value();
   auto grandchild2 = lineage_tree.lineage_id_lookup_map.getId("GRANDCHILD2").value();

   ASSERT_EQ(lineage_tree.lineage_tree.getParent(grandchild1).value(), child1);
   ASSERT_EQ(lineage_tree.lineage_tree.getParent(grandchild2).value(), child1);
   ASSERT_EQ(lineage_tree.lineage_tree.getParent(child1).value(), base);
   ASSERT_EQ(lineage_tree.lineage_tree.getParent(child2).value(), base);
   ASSERT_EQ(lineage_tree.lineage_tree.getParent(base), std::nullopt);
}

TEST(LineageTreeAndIdMap, correctCycleErrorInFile) {
   auto throwing_lambda = []() {
      LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAML(R"(
BASE:
  parents:
   - CHILD
CHILD:
  parents:
    - BASE
)"));
   };

   EXPECT_THAT(
      throwing_lambda,
      ThrowsMessage<silo::preprocessing::PreprocessingException>(
         ::testing::HasSubstr("The given LineageTree contains the cycle: BASE -> CHILD -> BASE")
      )
   );
}

TEST(LineageTreeAndIdMap, correctSelfCycleErrorInFile) {
   auto throwing_lambda = []() {
      LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAML(R"(
BASE:
  parents:
   - BASE
CHILD:
  parents:
    - BASE
)"));
   };

   EXPECT_THAT(
      throwing_lambda,
      ThrowsMessage<silo::preprocessing::PreprocessingException>(::testing::HasSubstr(
         "The given LineageTree contains the cycle: BASE -> BASE"

      ))
   );
}

TEST(LineageTreeAndIdMap, correctLassoCycleErrorInFile) {
   auto throwing_lambda = []() {
      LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAML(R"(
BASE: {}
CHILD1:
  parents:
    - BASE
    - CHILD3
CHILD2:
  parents:
    - CHILD1
CHILD3:
  parents:
    - CHILD2
)"));
   };

   EXPECT_THAT(
      throwing_lambda,
      ThrowsMessage<silo::preprocessing::PreprocessingException>(::testing::HasSubstr(
         "The given LineageTree contains the cycle: CHILD1 -> CHILD2 -> CHILD3 -> CHILD1"
      ))
   );
}

TEST(LineageDefinitionFile, errorOnDuplicateKey) {
   auto throwing_lambda = []() {
      LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAML(R"(
some_duplicate_lineage:
  parents:
  - some_other_key
some_other_key:
some_duplicate_lineage:
  parents:
  - some_other_key)"));
   };

   EXPECT_THAT(
      throwing_lambda,
      ThrowsMessage<silo::preprocessing::PreprocessingException>(::testing::HasSubstr(
         "The lineage definitions contain the duplicate lineage 'some_duplicate_lineage'"

      ))
   );
}

TEST(LineageDefinitionFile, errorOnDuplicateAlias) {
   auto throwing_lambda = []() {
      LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAML(R"(
lineage1:
  aliases:
  - duplicate_alias
  parents:
  - some_other_key
lineage2:
lineage3:
  aliases:
  - duplicate_alias
  parents:
  - some_other_key)"));
   };

   EXPECT_THAT(
      throwing_lambda,
      ThrowsMessage<silo::preprocessing::PreprocessingException>(::testing::HasSubstr(
         "The alias 'duplicate_alias' for lineage 'lineage3' is already defined as a lineage "
         "or another alias."
      ))
   );
}

TEST(LineageDefinitionFile, errorOnLineageAsAlias) {
   auto throwing_lambda = []() {
      LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAML(R"(
lineage1:
  aliases:
  - some_alias
  parents:
  - some_other_key
lineage2_also_used_as_alias:
lineage3:
  aliases:
  - lineage2_also_used_as_alias
  parents:
  - some_other_key)"));
   };

   EXPECT_THAT(
      throwing_lambda,
      ThrowsMessage<silo::preprocessing::PreprocessingException>(::testing::HasSubstr(
         "The alias 'lineage2_also_used_as_alias' for lineage 'lineage3' is already defined "
         "as a lineage or another alias."
      ))
   );
}

TEST(containsCycle, doesNotFindCycleInPangoLineageTree) {
   ASSERT_NO_THROW(LineageTreeAndIdMap::fromLineageDefinitionFilePath(
      "testBaseData/exampleDataset/lineage_definitions.yaml"
   ));
}

TEST(containsCycle, doesNotFindCycleInMediumSizedChainGraph) {
   std::vector<std::pair<uint32_t, uint32_t>> chain_edges;
   const uint32_t number_of_edges = UINT16_MAX;
   chain_edges.reserve(number_of_edges);
   for (size_t i = 0; i + 1 < number_of_edges; i++) {
      chain_edges.emplace_back(i, i + 1);
   }
   ASSERT_FALSE(silo::common::containsCycle(number_of_edges, chain_edges));
}

TEST(containsCycle, findsCycles) {
   // 1. Simple cycle
   ASSERT_TRUE(silo::common::containsCycle(3, {{0, 1}, {1, 0}}));

   // 2. Graph with a cycle and disconnected edges
   ASSERT_TRUE(silo::common::containsCycle(5, {{0, 1}, {1, 2}, {2, 0}, {3, 4}}));

   // 3. Complete graph of 3 nodes (cycle)
   ASSERT_TRUE(silo::common::containsCycle(3, {{0, 1}, {1, 2}, {2, 0}}));

   // 4. Cycle in a large graph
   ASSERT_TRUE(silo::common::containsCycle(6, {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 0}}));

   // 5. Disconnected cycle components
   ASSERT_TRUE(silo::common::containsCycle(7, {{0, 1}, {1, 2}, {2, 3}, {4, 5}, {5, 6}, {6, 4}}));

   // 6. Self-loop, cycle present
   ASSERT_TRUE(silo::common::containsCycle(3, {{0, 1}, {1, 2}, {2, 2}}));

   // 7. 4 nodes forming two separate cycles
   ASSERT_TRUE(silo::common::containsCycle(4, {{0, 1}, {1, 0}, {2, 3}, {3, 2}}));

   // 8. Tree structure with additional edge forming a cycle
   ASSERT_TRUE(silo::common::containsCycle(6, {{0, 1}, {0, 2}, {1, 3}, {4, 1}, {3, 5}, {5, 4}}));

   // 9. Fully connected graph of 4 nodes (cycle present)
   ASSERT_TRUE(silo::common::containsCycle(4, {{0, 1}, {2, 0}, {0, 3}, {1, 2}, {3, 1}, {2, 3}}));

   // 10. Graph with multiple isolated cycles
   ASSERT_TRUE(
      silo::common::containsCycle(8, {{0, 1}, {1, 2}, {2, 0}, {3, 4}, {4, 5}, {5, 3}, {6, 7}})
   );

   // 11. Single node with a self-loop (directed cycle)
   ASSERT_TRUE(silo::common::containsCycle(1, {{0, 0}}));
}

TEST(containsCycle, correctTrees) {
   // 1. No edges, no cycle
   ASSERT_FALSE(silo::common::containsCycle(5, {}));

   // 2. Single edge, no cycle
   ASSERT_FALSE(silo::common::containsCycle(2, {{0, 1}}));

   // 3. Linear chain of edges, no cycle
   ASSERT_FALSE(silo::common::containsCycle(4, {{0, 1}, {1, 2}, {2, 3}}));

   // 4. Multiple disconnected edges, no cycle
   ASSERT_FALSE(silo::common::containsCycle(6, {{0, 1}, {2, 3}, {4, 5}}));

   // 5. Tree structure, no cycle
   ASSERT_FALSE(silo::common::containsCycle(5, {{0, 1}, {0, 2}, {1, 3}, {1, 4}}));

   // 6. Graph with a single node and no edges
   ASSERT_FALSE(silo::common::containsCycle(1, {}));

   // 7. Two disconnected trees (no cycle)
   ASSERT_FALSE(silo::common::containsCycle(6, {{0, 1}, {0, 2}, {3, 4}, {3, 5}}));

   // 8. Star-shaped graph, no cycle
   ASSERT_FALSE(silo::common::containsCycle(5, {{0, 1}, {0, 2}, {0, 3}, {0, 4}}));

   // 9. Two disconnected nodes
   ASSERT_FALSE(silo::common::containsCycle(2, {}));

   // 10. Chain of 5 nodes
   ASSERT_FALSE(silo::common::containsCycle(5, {{0, 1}, {1, 2}, {2, 3}, {3, 4}}));
}

TEST(containsCycle, correctDirectedAcyclicGraphs) {
   // 1. Undirected lasso, but no directed cycle
   ASSERT_FALSE(silo::common::containsCycle(6, {{0, 1}, {0, 2}, {1, 3}, {1, 4}, {3, 5}, {4, 5}}));

   // 2. Chain of 5 nodes and first to last shortcut
   ASSERT_FALSE(silo::common::containsCycle(5, {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {0, 4}}));
}