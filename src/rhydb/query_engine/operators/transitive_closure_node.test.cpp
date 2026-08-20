#include <nlohmann/json.hpp>

#include "rhydb/preprocessing/lineage_definition_file.h"
#include "rhydb/test/query_fixture.test.h"

namespace {
using rhydb::ReferenceGenomes;
using rhydb::common::LineageTreeAndIdMap;
using rhydb::preprocessing::LineageDefinitionFile;
using rhydb::test::QueryTestData;
using rhydb::test::QueryTestScenario;

nlohmann::json createDataWithLineageValue(const std::string& primaryKey, const std::string& value) {
   return {
      {"primaryKey", primaryKey},
      {"pango_lineage", value},
      {"pango_lineage_indexed", value},
      {"segment1", nullptr},
      {"unaligned_segment1", nullptr},
      {"gene1", nullptr}
   };
}

nlohmann::json createDataWithLineageNullValue(const std::string& primaryKey) {
   return {
      {"primaryKey", primaryKey},
      {"pango_lineage", nullptr},
      {"pango_lineage_indexed", nullptr},
      {"segment1", nullptr},
      {"unaligned_segment1", nullptr},
      {"gene1", nullptr}
   };
}

const std::vector<nlohmann::json> DATA = {
   createDataWithLineageValue("id_0", "BASE.1"),
   createDataWithLineageValue("id_1", "CHILD"),
   createDataWithLineageValue("id_2", "CHILD"),
   createDataWithLineageValue("id_3", "CHILD.2"),
   createDataWithLineageValue("id_4", "GRANDCHILD"),
   createDataWithLineageNullValue("id_5"),
};

// Two lineage columns:
//   * `pango_lineage` is a plain STRING column (no index) that carries each sequence's lineage.
//     transitiveClosure emits STRING from/to columns, and join() requires matching column
//     types, so the value we join the closure against must itself be STRING.
//   * `pango_lineage_indexed` has `generateLineageIndex` + `lineageIndexType: table`, which
//     materializes the companion `pango_lineage_indexed` relation table (columns
//     `lineage`/`parent`) whose closure we compute. A lineage-indexed column is necessarily
//     dictionary-encoded (generateIndex is required), which is why it cannot double as the
//     STRING join key.
const auto DATABASE_CONFIG =
   R"(
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
    - name: "pango_lineage"
      type: "string"
    - name: "pango_lineage_indexed"
      type: "string"
      generateIndex: true
      generateLineageIndex: test_lineage_index
      lineageIndexType: table
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{"gene1", "*"}},
};

//   BASE.1
//   ├── CHILD
//   │   └── GRANDCHILD
//   └── CHILD.2
const auto LINEAGE_TREE =
   LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAMLString(R"(
BASE.1:
  parents: []
CHILD:
  parents:
  - BASE.1
CHILD.2:
  parents:
  - BASE.1
GRANDCHILD:
  parents:
  - CHILD
)"));

const QueryTestData TEST_DATA{
   .ndjson_input_data = DATA,
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .lineage_trees = {{"test_lineage_index", LINEAGE_TREE}}
};

// Plain transitive closure of the parent -> lineage edges: every (ancestor, descendant) pair.
const QueryTestScenario TRANSITIVE_CLOSURE_SCENARIO = {
   .name = "TRANSITIVE_CLOSURE_SCENARIO",
   .query = "pango_lineage_indexed.transitiveClosure('parent', 'lineage').orderBy({from, to})",
   .expected_query_result = nlohmann::json(
      {{{"from", "BASE.1"}, {"to", "CHILD"}},
       {{"from", "BASE.1"}, {"to", "CHILD.2"}},
       {{"from", "BASE.1"}, {"to", "GRANDCHILD"}},
       {{"from", "CHILD"}, {"to", "GRANDCHILD"}}}
   )
};

// Reflexive-transitive closure: additionally pairs every lineage with itself.
const QueryTestScenario TRANSITIVE_CLOSURE_INCLUDE_VERTICES_SCENARIO = {
   .name = "TRANSITIVE_CLOSURE_INCLUDE_VERTICES_SCENARIO",
   .query =
      "pango_lineage_indexed.transitiveClosure('parent', 'lineage', includeVertices:=true)"
      ".orderBy({from, to})",
   .expected_query_result = nlohmann::json(
      {{{"from", "BASE.1"}, {"to", "BASE.1"}},
       {{"from", "BASE.1"}, {"to", "CHILD"}},
       {{"from", "BASE.1"}, {"to", "CHILD.2"}},
       {{"from", "BASE.1"}, {"to", "GRANDCHILD"}},
       {{"from", "CHILD"}, {"to", "CHILD"}},
       {{"from", "CHILD"}, {"to", "GRANDCHILD"}},
       {{"from", "CHILD.2"}, {"to", "CHILD.2"}},
       {{"from", "GRANDCHILD"}, {"to", "GRANDCHILD"}}}
   )
};

// The motivating use case: count every lineage together with all of its sublineages.
// transitiveClosure (reflexive) -> join default on `to` = pango_lineage -> groupBy `from`.
//   BASE.1     : id_0, id_1, id_2, id_3, id_4 -> 5
//   CHILD      : id_1, id_2, id_4             -> 3
//   CHILD.2    : id_3                         -> 1
//   GRANDCHILD : id_4                         -> 1
const QueryTestScenario COUNT_LINEAGE_INCLUDING_SUBLINEAGES_SCENARIO = {
   .name = "COUNT_LINEAGE_INCLUDING_SUBLINEAGES_SCENARIO",
   .query =
      "pango_lineage_indexed.transitiveClosure('parent', 'lineage', includeVertices:=true)"
      ".join(default, to = pango_lineage)"
      ".groupBy({count := count()}, {from})"
      ".orderBy({from})",
   .expected_query_result = nlohmann::json(
      {{{"from", "BASE.1"}, {"count", 5}},
       {{"from", "CHILD"}, {"count", 3}},
       {{"from", "CHILD.2"}, {"count", 1}},
       {{"from", "GRANDCHILD"}, {"count", 1}}}
   )
};

// The input is any relation-producing subquery, not just a bare table: here a project() feeds
// transitiveClosure the two edge columns it reads.
const QueryTestScenario SUBQUERY_INPUT_SCENARIO = {
   .name = "SUBQUERY_INPUT_SCENARIO",
   .query =
      "pango_lineage_indexed.project({parent, lineage})"
      ".transitiveClosure('parent', 'lineage').orderBy({from, to})",
   .expected_query_result = nlohmann::json(
      {{{"from", "BASE.1"}, {"to", "CHILD"}},
       {{"from", "BASE.1"}, {"to", "CHILD.2"}},
       {{"from", "BASE.1"}, {"to", "GRANDCHILD"}},
       {{"from", "CHILD"}, {"to", "GRANDCHILD"}}}
   )
};

// The closure is emitted in `materialization_cutoff + 1`-sized batches; with a cutoff of 0 every
// pair ends up in a batch of its own, which exercises the streaming path across several batches.
const QueryTestScenario TRANSITIVE_CLOSURE_ONE_PAIR_PER_BATCH_SCENARIO = {
   .name = "TRANSITIVE_CLOSURE_ONE_PAIR_PER_BATCH_SCENARIO",
   .query =
      "pango_lineage_indexed.transitiveClosure('parent', 'lineage', includeVertices:=true)"
      ".orderBy({from, to})",
   .expected_query_result = nlohmann::json(
      {{{"from", "BASE.1"}, {"to", "BASE.1"}},
       {{"from", "BASE.1"}, {"to", "CHILD"}},
       {{"from", "BASE.1"}, {"to", "CHILD.2"}},
       {{"from", "BASE.1"}, {"to", "GRANDCHILD"}},
       {{"from", "CHILD"}, {"to", "CHILD"}},
       {{"from", "CHILD"}, {"to", "GRANDCHILD"}},
       {{"from", "CHILD.2"}, {"to", "CHILD.2"}},
       {{"from", "GRANDCHILD"}, {"to", "GRANDCHILD"}}}
   ),
   .query_options = rhydb::config::QueryOptions{.materialization_cutoff = 0}
};

const QueryTestScenario UNKNOWN_COLUMN_SCENARIO = {
   .name = "UNKNOWN_COLUMN_SCENARIO",
   .query = "pango_lineage_indexed.transitiveClosure('parent', 'does_not_exist')",
   .expected_error_message =
      "transitiveClosure() column 'does_not_exist' is not present in the input's output schema"
};

}  // namespace

QUERY_TEST(
   TransitiveClosureTest,
   TEST_DATA,
   ::testing::Values(
      TRANSITIVE_CLOSURE_SCENARIO,
      TRANSITIVE_CLOSURE_INCLUDE_VERTICES_SCENARIO,
      TRANSITIVE_CLOSURE_ONE_PAIR_PER_BATCH_SCENARIO,
      COUNT_LINEAGE_INCLUDING_SUBLINEAGES_SCENARIO,
      SUBQUERY_INPUT_SCENARIO,
      UNKNOWN_COLUMN_SCENARIO
   )
);
