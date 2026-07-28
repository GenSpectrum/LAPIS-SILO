#include <nlohmann/json.hpp>

#include "silo/preprocessing/lineage_definition_file.h"
#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::common::LineageTreeAndIdMap;
using silo::preprocessing::LineageDefinitionFile;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

// Fixture mirrors expressions/lineage_filter.test.cpp (BASE.1 / CHILD / CHILD.2 / RECOMBINANT),
// with an alias `CH_ALIAS` added on CHILD to regression-test that aliases are NOT double-emitted.
//
//        BASE.1
//       /      \
//    CHILD    CHILD.2
//       \      /
//      RECOMBINANT
//
// Observed rows: BASE.1 x2 (id_0, id_1), CHILD x1 (id_2), null (id_3), RECOMBINANT x1 (id_4).

nlohmann::json createDataWithLineageValue(const std::string& primaryKey, std::string value) {
   return {
      {"primaryKey", primaryKey},
      {"pango_lineage", value},
      {"float_value", nullptr},
      {"segment1", nullptr},
      {"unaligned_segment1", nullptr},
      {"gene1", nullptr}
   };
}

nlohmann::json createDataWithLineageNullValue(const std::string& primaryKey) {
   return {
      {"primaryKey", primaryKey},
      {"pango_lineage", nullptr},
      {"float_value", nullptr},
      {"segment1", nullptr},
      {"unaligned_segment1", nullptr},
      {"gene1", nullptr}
   };
}

const std::vector<nlohmann::json> DATA = {
   createDataWithLineageValue("id_0", "BASE.1"),
   createDataWithLineageValue("id_1", "BASE.1"),
   createDataWithLineageValue("id_2", "CHILD"),
   createDataWithLineageNullValue("id_3"),
   createDataWithLineageValue("id_4", "RECOMBINANT")
};

const auto DATABASE_CONFIG =
   R"(
defaultNucleotideSequence: "segment1"
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
    - name: "pango_lineage"
      type: "string"
      generateIndex: true
      generateLineageIndex: test_lineage_index
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{"gene1", "*"}},
};

const auto LINEAGE_TREE =
   LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAMLString(R"(
CHILD:
  aliases:
  - CH_ALIAS
  parents:
  - BASE.1
CHILD.2:
  parents:
  - BASE.1
BASE.1:
  parents: []
RECOMBINANT:
  parents:
  - CHILD
  - CHILD.2
)"));

const QueryTestData TEST_DATA{
   .ndjson_input_data = DATA,
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .lineage_trees = {{"test_lineage_index", LINEAGE_TREE}}
};

// Exact SaneQL contract for the sublineage-inclusive groupBy (this is what the LAPIS PR must emit).
const std::string GROUP_BY =
   "groupBy({count := count()}, {lineage(pango_lineage, includeSublineages := true)})";

// No filter, default recombinant mode (doNotFollow). Every defined lineage is emitted, including
// the zero-count CHILD.2 and never the alias CH_ALIAS. RECOMBINANT is not rolled up into its
// parents (doNotFollow), so BASE.1 = {id_0,id_1,id_2} = 3.
const QueryTestScenario BASE_INCLUDES_SUBLINEAGES = {
   .name = "BASE_INCLUDES_SUBLINEAGES",
   .query = "default." + GROUP_BY + ".orderBy({pango_lineage})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"pango_lineage":"BASE.1","count":3},
      {"pango_lineage":"CHILD","count":1},
      {"pango_lineage":"CHILD.2","count":0},
      {"pango_lineage":"RECOMBINANT","count":1}
   ])")
};

// alwaysFollow: the recombinant row id_4 is counted under both parents and BASE.1.
const QueryTestScenario ALWAYS_FOLLOW = {
   .name = "ALWAYS_FOLLOW",
   .query =
      "default.groupBy({count := count()}, {lineage(pango_lineage, includeSublineages := true, "
      "recombinantFollowingMode := 'alwaysFollow')}).orderBy({pango_lineage})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"pango_lineage":"BASE.1","count":4},
      {"pango_lineage":"CHILD","count":2},
      {"pango_lineage":"CHILD.2","count":1},
      {"pango_lineage":"RECOMBINANT","count":1}
   ])")
};

// followIfFullyContainedInClade: id_4 rolls up only to its clade ancestor BASE.1, not to the
// individual parents CHILD / CHILD.2.
const QueryTestScenario FOLLOW_IF_CONTAINED = {
   .name = "FOLLOW_IF_CONTAINED",
   .query =
      "default.groupBy({count := count()}, {lineage(pango_lineage, includeSublineages := true, "
      "recombinantFollowingMode := 'followIfFullyContainedInClade')}).orderBy({pango_lineage})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"pango_lineage":"BASE.1","count":4},
      {"pango_lineage":"CHILD","count":1},
      {"pango_lineage":"CHILD.2","count":0},
      {"pango_lineage":"RECOMBINANT","count":1}
   ])")
};

// Explicit doNotFollow matches the default.
const QueryTestScenario EXPLICIT_DO_NOT_FOLLOW = {
   .name = "EXPLICIT_DO_NOT_FOLLOW",
   .query =
      "default.groupBy({count := count()}, {lineage(pango_lineage, includeSublineages := true, "
      "recombinantFollowingMode := 'doNotFollow')}).orderBy({pango_lineage})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"pango_lineage":"BASE.1","count":3},
      {"pango_lineage":"CHILD","count":1},
      {"pango_lineage":"CHILD.2","count":0},
      {"pango_lineage":"RECOMBINANT","count":1}
   ])")
};

// A filter below the groupBy: only the two exact BASE.1 rows survive, so counts reflect the
// intersection with each lineage's subtree.
const QueryTestScenario WITH_FILTER = {
   .name = "WITH_FILTER",
   .query = "default.filter(pango_lineage.lineage('BASE.1'))." + GROUP_BY + ".orderBy({pango_lineage})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"pango_lineage":"BASE.1","count":2},
      {"pango_lineage":"CHILD","count":0},
      {"pango_lineage":"CHILD.2","count":0},
      {"pango_lineage":"RECOMBINANT","count":0}
   ])")
};

// orderBy + limit works over the emitted rows.
const QueryTestScenario WITH_ORDER_AND_LIMIT = {
   .name = "WITH_ORDER_AND_LIMIT",
   .query = "default." + GROUP_BY + ".orderBy({pango_lineage}).limit(2)",
   .expected_query_result = nlohmann::json::parse(R"([
      {"pango_lineage":"BASE.1","count":3},
      {"pango_lineage":"CHILD","count":1}
   ])")
};

const QueryTestScenario ERROR_EXTRA_GROUP_COLUMN = {
   .name = "ERROR_EXTRA_GROUP_COLUMN",
   .query =
      "default.groupBy({count := count()}, {lineage(pango_lineage, includeSublineages := true), "
      "primaryKey})",
   .expected_error_message =
      "a lineage(...) group-by column must be the only group-by column; it cannot be combined "
      "with other group-by columns"
};

const QueryTestScenario ERROR_MULTIPLE_AGGREGATES = {
   .name = "ERROR_MULTIPLE_AGGREGATES",
   .query =
      "default.groupBy({count := count(), other := count()}, {lineage(pango_lineage, "
      "includeSublineages := true)})",
   .expected_error_message =
      "a lineage(...) group-by supports exactly one aggregate, which must be count()"
};

const QueryTestScenario ERROR_NON_LINEAGE_COLUMN = {
   .name = "ERROR_NON_LINEAGE_COLUMN",
   .query = "default.groupBy({count := count()}, {lineage(primaryKey, includeSublineages := true)})",
   .expected_error_message =
      "lineage aggregation cannot be applied to column 'primaryKey' as it is not of type indexed "
      "string"
};

const QueryTestScenario ERROR_REQUIRES_INCLUDE_SUBLINEAGES = {
   .name = "ERROR_REQUIRES_INCLUDE_SUBLINEAGES",
   .query = "default.groupBy({count := count()}, {lineage(pango_lineage)})",
   .expected_error_message =
      "lineage(...) as a group-by column requires 'includeSublineages := true'. For exact "
      "grouping use the bare column name 'pango_lineage' instead."
};

}  // namespace

QUERY_TEST(
   LineageAggregateNodeTest,
   TEST_DATA,
   ::testing::Values(
      BASE_INCLUDES_SUBLINEAGES,
      ALWAYS_FOLLOW,
      FOLLOW_IF_CONTAINED,
      EXPLICIT_DO_NOT_FOLLOW,
      WITH_FILTER,
      WITH_ORDER_AND_LIMIT,
      ERROR_EXTRA_GROUP_COLUMN,
      ERROR_MULTIPLE_AGGREGATES,
      ERROR_NON_LINEAGE_COLUMN,
      ERROR_REQUIRES_INCLUDE_SUBLINEAGES
   )
)
