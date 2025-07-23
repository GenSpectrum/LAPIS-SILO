#include <nlohmann/json.hpp>

#include <optional>

#include "silo/preprocessing/lineage_definition_file.h"
#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::common::LineageTreeAndIdMap;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::preprocessing::LineageDefinitionFile;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

const std::string SOME_BASE_LINEAGE = "BASE.1";
const std::string SOME_SUBLINEAGE = "CHILD";
const std::string RECOMBINANT_LINEAGE = "RECOMBINANT";

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
   createDataWithLineageValue("id_0", SOME_BASE_LINEAGE),
   createDataWithLineageValue("id_1", SOME_BASE_LINEAGE),
   createDataWithLineageValue("id_2", SOME_SUBLINEAGE),
   createDataWithLineageNullValue("id_3"),
   createDataWithLineageValue("id_4", RECOMBINANT_LINEAGE)
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
      generateLineageIndex: true
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{"gene1", "*"}},
};

const auto LINEAGE_TREE =
   LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAMLString(R"(
CHILD:
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
   .lineage_tree = LINEAGE_TREE
};

nlohmann::json createLineageQuery(const nlohmann::json value, bool include_sublineages) {
   return {
      {"action", {{"type", "Details"}}},
      {"filterExpression",
       {{"type", "Lineage"},
        {"column", "pango_lineage"},
        {"value", value},
        {"includeSublineages", include_sublineages}}}
   };
}

const QueryTestScenario LINEAGE_FILTER_SCENARIO = {
   .name = "lineageFilter",
   .query = createLineageQuery(SOME_BASE_LINEAGE, false),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"pango_lineage", SOME_BASE_LINEAGE}},
       {{"primaryKey", "id_1"}, {"pango_lineage", SOME_BASE_LINEAGE}}}
   )
};

const QueryTestScenario LINEAGE_FILTER_INCLUDING_SUBLINEAGES_SCENARIO = {
   .name = "lineageFilterIncludingSublineages",
   .query = createLineageQuery(SOME_BASE_LINEAGE, true),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"pango_lineage", SOME_BASE_LINEAGE}},
       {{"primaryKey", "id_1"}, {"pango_lineage", SOME_BASE_LINEAGE}},
       {{"primaryKey", "id_2"}, {"pango_lineage", SOME_SUBLINEAGE}}}
   )
};

const QueryTestScenario LINEAGE_FILTER_NULL_SCENARIO = {
   .name = "lineageFilterNull",
   .query = createLineageQuery(nullptr, false),
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_3"}, {"pango_lineage", nullptr}}})
};

const QueryTestScenario LINEAGE_FILTER_NULL_INCLUDING_SUBLINEAGES_SCENARIO = {
   .name = "lineageFilterNullIncludingSublineages",
   .query = createLineageQuery(nullptr, true),
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_3"}, {"pango_lineage", nullptr}}})
};

const QueryTestScenario FILTER_INCLUDING_RECOMBINANTS = {
   .name = "FILTER_INCLUDING_RECOMBINANTS",
   .query = nlohmann::json::parse(R"(
{
  "action": {"type": "Details"},
  "filterExpression": {
    "type": "Lineage",
    "column": "pango_lineage",
    "value": "CHILD",
    "includeSublineages": true,
    "recombinantFollowingMode": "alwaysFollow"
  }
})"),
   .expected_query_result = nlohmann::json::parse(R"(
[{"pango_lineage":"CHILD","primaryKey":"id_2"},
{"pango_lineage":"RECOMBINANT","primaryKey":"id_4"}]
)")
};

const QueryTestScenario FILTER_INCLUDING_CONTAINED_RECOMBINANTS = {
   .name = "FILTER_INCLUDING_CONTAINED_RECOMBINANTS",
   .query = nlohmann::json::parse(R"(
{
  "action": {"type": "Details"},
  "filterExpression": {
    "type": "Lineage",
    "column": "pango_lineage",
    "value": "BASE.1",
    "includeSublineages": true,
    "recombinantFollowingMode": "followIfFullyContainedInClade"
  }
})"),
   .expected_query_result = nlohmann::json::parse(R"(
[{"pango_lineage":"BASE.1","primaryKey":"id_0"},
{"pango_lineage":"BASE.1","primaryKey":"id_1"},
{"pango_lineage":"CHILD","primaryKey":"id_2"},
{"pango_lineage":"RECOMBINANT","primaryKey":"id_4"}]
)")
};

const QueryTestScenario DOES_NOT_FILTER_NON_INCLUDED_RECOMBINANTS = {
   .name = "DOES_NOT_FILTER_NON_INCLUDED_RECOMBINANTS",
   .query = nlohmann::json::parse(R"(
{
  "action": {"type": "Details"},
  "filterExpression": {
    "type": "Lineage",
    "column": "pango_lineage",
    "value": "CHILD",
    "includeSublineages": true,
    "recombinantFollowingMode": "followIfFullyContainedInClade"
  }
})"),
   .expected_query_result = nlohmann::json::parse(R"(
[{"pango_lineage":"CHILD","primaryKey":"id_2"}]
)")
};

const QueryTestScenario EXPLICIT_DO_NOT_FOLLOW = {
   .name = "EXPLICIT_DO_NOT_FOLLOW",
   .query = nlohmann::json::parse(R"(
{
  "action": {"type": "Details"},
  "filterExpression": {
    "type": "Lineage",
    "column": "pango_lineage",
    "value": "BASE.1",
    "includeSublineages": true,
    "recombinantFollowingMode": "doNotFollow"
  }
})"),
   .expected_query_result = nlohmann::json::parse(R"(
[{"pango_lineage":"BASE.1","primaryKey":"id_0"},
{"pango_lineage":"BASE.1","primaryKey":"id_1"},
{"pango_lineage":"CHILD","primaryKey":"id_2"}]
)")
};

}  // namespace

QUERY_TEST(
   LineageFilterTest,
   TEST_DATA,
   ::testing::Values(
      LINEAGE_FILTER_SCENARIO,
      LINEAGE_FILTER_INCLUDING_SUBLINEAGES_SCENARIO,
      LINEAGE_FILTER_NULL_SCENARIO,
      LINEAGE_FILTER_NULL_INCLUDING_SUBLINEAGES_SCENARIO,
      FILTER_INCLUDING_RECOMBINANTS,
      FILTER_INCLUDING_CONTAINED_RECOMBINANTS,
      DOES_NOT_FILTER_NON_INCLUDED_RECOMBINANTS,
      EXPLICIT_DO_NOT_FOLLOW
   )
)
