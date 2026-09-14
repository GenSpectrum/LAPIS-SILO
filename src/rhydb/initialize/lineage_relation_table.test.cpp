#include "rhydb/initialize/lineage_relation_table.h"

#include <optional>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "rhydb/common/lineage_tree.h"
#include "rhydb/preprocessing/lineage_definition_file.h"
#include "rhydb/storage/reference_genomes.h"
#include "rhydb/test/query_fixture.test.h"

using rhydb::common::LineageTreeAndIdMap;
using rhydb::initialize::buildLineageAliasRows;
using rhydb::initialize::buildLineageRelationRows;
using rhydb::initialize::LineageAliasRow;
using rhydb::initialize::LineageRelationRow;
using rhydb::preprocessing::LineageDefinitionFile;
using ::testing::UnorderedElementsAreArray;

namespace {
LineageRelationRow edge(
   std::string lineage,
   std::optional<std::string> parent,
   bool is_recombinant_edge = false,
   std::optional<std::string> recombinant_clade_ancestor = std::nullopt
) {
   return LineageRelationRow{
      .lineage = std::move(lineage),
      .parent = std::move(parent),
      .is_recombinant_edge = is_recombinant_edge,
      .recombinant_clade_ancestor = std::move(recombinant_clade_ancestor)
   };
}
}  // namespace

TEST(LineageRelationTable, emitsOnlyDirectEdgesForALinearChain) {
   auto tree = LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAMLString(
      R"(
BASE:
  parents: []
CHILD:
  parents:
    - BASE
GRANDCHILD:
  parents:
    - CHILD
)"
   ));

   const auto rows = buildLineageRelationRows(tree);

   // No transitive closure: GRANDCHILD points only at CHILD, not at BASE. The root gets a
   // parent-less row.
   EXPECT_THAT(
      rows,
      UnorderedElementsAreArray(
         {edge("BASE", std::nullopt), edge("CHILD", "BASE"), edge("GRANDCHILD", "CHILD")}
      )
   );
}

TEST(LineageRelationTable, recombinantEmitsOneEdgePerParentWithCladeAncestor) {
   // XBB recombines A.1 and A.2, whose clade ancestor is A.
   auto tree = LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAMLString(
      R"(
A:
  parents: []
A.1:
  parents:
    - A
A.2:
  parents:
    - A
XBB:
  parents:
    - A.1
    - A.2
)"
   ));

   const auto rows = buildLineageRelationRows(tree);

   EXPECT_THAT(
      rows,
      UnorderedElementsAreArray(
         {edge("A", std::nullopt),
          edge("A.1", "A"),
          edge("A.2", "A"),
          edge("XBB", "A.1", /*is_recombinant_edge=*/true, /*recombinant_clade_ancestor=*/"A"),
          edge("XBB", "A.2", /*is_recombinant_edge=*/true, /*recombinant_clade_ancestor=*/"A")}
      )
   );
}

TEST(LineageRelationTable, aliasRowsMapEachAliasToItsCanonicalLineage) {
   auto tree = LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAMLString(
      R"(
BASE:
  aliases:
  - B
CHILD:
  aliases:
  - C1
  - C2
  parents:
    - BASE
)"
   ));

   EXPECT_THAT(
      buildLineageAliasRows(tree),
      UnorderedElementsAreArray(
         {LineageAliasRow{.alias = "B", .lineage = "BASE"},
          LineageAliasRow{.alias = "C1", .lineage = "CHILD"},
          LineageAliasRow{.alias = "C2", .lineage = "CHILD"}}
      )
   );
   // The aliases stay out of the edges: they are names, not lineages.
   EXPECT_THAT(
      buildLineageRelationRows(tree),
      UnorderedElementsAreArray({edge("BASE", std::nullopt), edge("CHILD", "BASE")})
   );
}

TEST(LineageRelationTable, aliasRowsAreEmptyWithoutAliases) {
   auto tree = LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAMLString(
      R"(
BASE:
  parents: []
)"
   ));

   EXPECT_TRUE(buildLineageAliasRows(tree).empty());
}

namespace {
using rhydb::ReferenceGenomes;
using rhydb::test::QueryTestData;
using rhydb::test::QueryTestScenario;

// A column with a lineage definition gets a companion relation table named after the column.
const auto DATABASE_CONFIG =
   R"(
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
    - name: "lin"
      type: "string"
      generateIndex: true
      generateLineageIndex: test_lineage_index
    - name: "other_lin"
      type: "string"
      generateIndex: true
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{"gene1", "*"}},
};

const auto QUERYABLE_LINEAGE_TREE =
   LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAMLString(R"(
BASE:
  parents: []
CHILD:
  aliases:
    - KID
  parents:
    - BASE
)"));

nlohmann::json createDataWithLineageValue(const std::string& primary_key, std::string value) {
   return {
      {"primaryKey", primary_key},
      {"other_lin", value},
      {"lin", std::move(value)},
      {"segment1", nullptr},
      {"unaligned_segment1", nullptr},
      {"gene1", nullptr}
   };
}

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {createDataWithLineageValue("id_1", "CHILD"), createDataWithLineageValue("id_2", "BASE")},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .lineage_trees = {{"test_lineage_index", QUERYABLE_LINEAGE_TREE}}
};

// Only the direct edges are stored (no transitive closure, no per-mode duplication): the root BASE
// (parent null) and the edge CHILD->BASE => 2 rows.
const QueryTestScenario RELATION_TABLE_CONTAINS_ONLY_DIRECT_EDGES = {
   .name = "RELATION_TABLE_CONTAINS_ONLY_DIRECT_EDGES",
   .query = "lin.groupBy({count:=count()})",
   .expected_query_result = nlohmann::json::parse(R"([{"count":2}])")
};

// The `parent` column is directly queryable: exactly one lineage (CHILD) has BASE as its direct
// parent.
const QueryTestScenario RELATION_TABLE_PARENT_IS_QUERYABLE = {
   .name = "RELATION_TABLE_PARENT_IS_QUERYABLE",
   .query = "lin.filter(parent = 'BASE').groupBy({count:=count()})",
   .expected_query_result = nlohmann::json::parse(R"([{"count":1}])")
};

const QueryTestScenario RELATION_TABLE_ROW_SHAPE = {
   .name = "RELATION_TABLE_ROW_SHAPE",
   .query =
      "lin.filter(lineage = 'CHILD').project({lineage, parent, is_recombinant_edge, "
      "recombinant_clade_ancestor})",
   .expected_query_result = nlohmann::json::parse(R"(
[{"lineage":"CHILD","parent":"BASE","is_recombinant_edge":false,
"recombinant_clade_ancestor":null}]
)")
};

// The aliases live in their own companion table, named after the column.
const QueryTestScenario ALIAS_TABLE_IS_QUERYABLE = {
   .name = "ALIAS_TABLE_IS_QUERYABLE",
   .query = "lin_aliases.project({alias, lineage})",
   .expected_query_result = nlohmann::json::parse(R"([{"alias":"KID","lineage":"CHILD"}])")
};

// A lineage may be queried by any of its aliases, with and without sublineages.
const QueryTestScenario ALIAS_RESOLVES_IN_LINEAGE_FILTER = {
   .name = "ALIAS_RESOLVES_IN_LINEAGE_FILTER",
   .query = "default.filter(lin.lineage('KID')).groupBy({count:=count()})",
   .expected_query_result = nlohmann::json::parse(R"([{"count":1}])")
};

const QueryTestScenario ALIAS_RESOLVES_WITH_SUBLINEAGES = {
   .name = "ALIAS_RESOLVES_WITH_SUBLINEAGES",
   .query =
      "default.filter(lin.lineage('KID', includeSublineages:=true)).groupBy({count:=count()})",
   .expected_query_result = nlohmann::json::parse(R"([{"count":1}])")
};

// `lineageDefinition` names the lineage system to resolve against. `other_lin` has no definition
// of its own, so filtering it hierarchically is only possible by naming one.
const QueryTestScenario LINEAGE_DEFINITION_NAMES_THE_SYSTEM_TO_RESOLVE_AGAINST = {
   .name = "LINEAGE_DEFINITION_NAMES_THE_SYSTEM_TO_RESOLVE_AGAINST",
   .query =
      "default.filter(other_lin.lineage('BASE', includeSublineages:=true, lineageDefinition:=lin))"
      ".groupBy({count:=count()})",
   .expected_query_result = nlohmann::json::parse(R"([{"count":2}])")
};

// Aliases resolve through the named definition too.
const QueryTestScenario LINEAGE_DEFINITION_RESOLVES_ALIASES = {
   .name = "LINEAGE_DEFINITION_RESOLVES_ALIASES",
   .query =
      "default.filter(other_lin.lineage('KID', lineageDefinition:=lin)).groupBy({count:=count()})",
   .expected_query_result = nlohmann::json::parse(R"([{"count":1}])")
};

const QueryTestScenario LINEAGE_DEFINITION_THAT_IS_NOT_ONE = {
   .name = "LINEAGE_DEFINITION_THAT_IS_NOT_ONE",
   .query =
      "default.filter(lin.lineage('BASE', includeSublineages:=true, lineageDefinition:=default))",
   .expected_error_message = "'default' is not a lineage definition of this database."
};

}  // namespace

QUERY_TEST(
   LineageRelationTableQueryTest,
   TEST_DATA,
   ::testing::Values(
      RELATION_TABLE_CONTAINS_ONLY_DIRECT_EDGES,
      RELATION_TABLE_PARENT_IS_QUERYABLE,
      RELATION_TABLE_ROW_SHAPE,
      ALIAS_TABLE_IS_QUERYABLE,
      ALIAS_RESOLVES_IN_LINEAGE_FILTER,
      ALIAS_RESOLVES_WITH_SUBLINEAGES,
      LINEAGE_DEFINITION_NAMES_THE_SYSTEM_TO_RESOLVE_AGAINST,
      LINEAGE_DEFINITION_RESOLVES_ALIASES,
      LINEAGE_DEFINITION_THAT_IS_NOT_ONE
   )
)
