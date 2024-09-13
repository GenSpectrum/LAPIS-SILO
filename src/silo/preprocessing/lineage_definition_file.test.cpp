#include "silo/preprocessing/lineage_definition_file.h"

#include <gtest/gtest.h>
#include <yaml-cpp/exceptions.h>

#include "silo/preprocessing/preprocessing_exception.h"

using silo::preprocessing::LineageDefinitionFile;

TEST(LineageDefinitionFile, correctlyParsesFromYAML) {
   auto lineage_definition_file = LineageDefinitionFile::fromYAML(R"(
BASE:
  parents: []
CHILD:
  parents:
    - BASE
)");
   ASSERT_EQ(lineage_definition_file.lineages.size(), 2);
   ASSERT_EQ(lineage_definition_file.lineages.at(0).lineage_name.string, "BASE");
   ASSERT_TRUE(lineage_definition_file.lineages.at(0).parent_lineages.empty());
   ASSERT_EQ(lineage_definition_file.lineages.at(1).lineage_name.string, "CHILD");
   ASSERT_EQ(lineage_definition_file.lineages.at(1).parent_lineages.size(), 1);
   ASSERT_EQ(lineage_definition_file.lineages.at(1).parent_lineages.at(0).string, "BASE");
}

TEST(LineageDefinitionFile, errorOnBadFormat) {
   ASSERT_THROW(
      LineageDefinitionFile::fromYAML(R"(
SOME_lineage:
-:
- does_not_exist
)"),
      YAML::InvalidNode
   );  // TODO improve error
}
