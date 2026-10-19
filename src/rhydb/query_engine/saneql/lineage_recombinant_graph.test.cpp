#include <map>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "rhydb/preprocessing/lineage_definition_file.h"
#include "rhydb/test/query_fixture.test.h"

namespace {
using rhydb::ReferenceGenomes;
using rhydb::common::LineageTreeAndIdMap;
using rhydb::preprocessing::LineageDefinitionFile;
using rhydb::test::QueryTestData;
using rhydb::test::QueryTestScenario;

/* A double diamond with the recombinants L4 (of L2 and L3) and L6 (of the recombinant L4 and L5),
 * below ROOT, which no row carries:
 *
 *     ROOT
 *      |
 *      L1
 *     /  \
 *   L2    L3
 *    \   /  \
 *     L4     L5
 *       \   /
 *        L6
 */
const auto LINEAGE_TREE =
   LineageTreeAndIdMap::fromLineageDefinitionFile(LineageDefinitionFile::fromYAMLString(R"(
ROOT:
  parents: []
L1:
  parents: [ROOT]
L2:
  parents: [L1]
L3:
  parents: [L1]
L4:
  parents: [L2, L3]
L5:
  parents: [L3]
L6:
  parents: [L4, L5]
)"));

// The lineage of each row, whose primary key id_<row>_<lineage> carries it.
const std::vector<std::string> ROW_LINEAGES =
   {"L1", "L4", "L1", "L2", "L3", "L6", "L5", "L6", "L5", "L5"};

std::string primaryKey(size_t row) {
   return fmt::format("id_{}_{}", row, ROW_LINEAGES.at(row));
}

std::vector<nlohmann::json> createData() {
   std::vector<nlohmann::json> data;
   data.reserve(ROW_LINEAGES.size());
   for (size_t row = 0; row < ROW_LINEAGES.size(); ++row) {
      data.push_back({
         {"primaryKey", primaryKey(row)},
         {"pango_lineage", ROW_LINEAGES.at(row)},
         {"segment1", nullptr},
         {"unaligned_segment1", nullptr},
         {"gene1", nullptr},
      });
   }
   return data;
}

const auto DATABASE_CONFIG =
   R"(
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

const QueryTestData TEST_DATA{
   .ndjson_input_data = createData(),
   .database_config = DATABASE_CONFIG,
   .reference_genomes = ReferenceGenomes{{{"segment1", "A"}}, {{"gene1", "*"}}},
   .lineage_trees = {{"test_lineage_index", LINEAGE_TREE}},
};

const std::vector<std::string> ALL_ROWS = [] {
   std::vector<std::string> rows;
   rows.reserve(ROW_LINEAGES.size());
   for (size_t row = 0; row < ROW_LINEAGES.size(); ++row) {
      rows.push_back(primaryKey(row));
   }
   return rows;
}();

// For each queried lineage, the primary keys of the matching rows (in insertion order) without
// sublineages and with sublineages in each recombinantFollowingMode.
struct ExpectedRows {
   std::vector<std::string> exact;
   std::vector<std::string> do_not_follow;
   std::vector<std::string> follow_if_fully_contained_in_clade;
   std::vector<std::string> always_follow;
};

const std::map<std::string, ExpectedRows> EXPECTED_ROWS = {
   {
      "ROOT",
      {
         .exact = {},
         .do_not_follow =
            {"id_0_L1", "id_2_L1", "id_3_L2", "id_4_L3", "id_6_L5", "id_8_L5", "id_9_L5"},
         .follow_if_fully_contained_in_clade = ALL_ROWS,
         .always_follow = ALL_ROWS,
      },
   },
   {
      "L1",
      {
         .exact = {"id_0_L1", "id_2_L1"},
         .do_not_follow =
            {"id_0_L1", "id_2_L1", "id_3_L2", "id_4_L3", "id_6_L5", "id_8_L5", "id_9_L5"},
         .follow_if_fully_contained_in_clade = ALL_ROWS,
         .always_follow = ALL_ROWS,
      },
   },
   {
      "L2",
      {
         .exact = {"id_3_L2"},
         .do_not_follow = {"id_3_L2"},
         .follow_if_fully_contained_in_clade = {"id_3_L2"},
         .always_follow = {"id_1_L4", "id_3_L2", "id_5_L6", "id_7_L6"},
      },
   },
   {
      "L3",
      {
         .exact = {"id_4_L3"},
         .do_not_follow = {"id_4_L3", "id_6_L5", "id_8_L5", "id_9_L5"},
         .follow_if_fully_contained_in_clade = {"id_4_L3", "id_6_L5", "id_8_L5", "id_9_L5"},
         .always_follow =
            {"id_1_L4", "id_4_L3", "id_5_L6", "id_6_L5", "id_7_L6", "id_8_L5", "id_9_L5"},
      },
   },
   {
      "L4",
      {
         .exact = {"id_1_L4"},
         .do_not_follow = {"id_1_L4"},
         .follow_if_fully_contained_in_clade = {"id_1_L4"},
         .always_follow = {"id_1_L4", "id_5_L6", "id_7_L6"},
      },
   },
   {
      "L5",
      {
         .exact = {"id_6_L5", "id_8_L5", "id_9_L5"},
         .do_not_follow = {"id_6_L5", "id_8_L5", "id_9_L5"},
         .follow_if_fully_contained_in_clade = {"id_6_L5", "id_8_L5", "id_9_L5"},
         .always_follow = {"id_5_L6", "id_6_L5", "id_7_L6", "id_8_L5", "id_9_L5"},
      },
   },
   {
      "L6",
      {
         .exact = {"id_5_L6", "id_7_L6"},
         .do_not_follow = {"id_5_L6", "id_7_L6"},
         .follow_if_fully_contained_in_clade = {"id_5_L6", "id_7_L6"},
         .always_follow = {"id_5_L6", "id_7_L6"},
      },
   },
};

QueryTestScenario scenario(
   const std::string& name,
   const std::string& lineage,
   const std::string& arguments,
   const std::vector<std::string>& primary_keys
) {
   nlohmann::json expected = nlohmann::json::array();
   for (const auto& primary_key : primary_keys) {
      expected.push_back({{"primaryKey", primary_key}});
   }
   return {
      .name = fmt::format("{}_{}", lineage, name),
      .query = fmt::format(
         "data.filter(pango_lineage.lineage('{}'{})).project({{primaryKey}})", lineage, arguments
      ),
      .expected_query_result = expected,
   };
}

std::vector<QueryTestScenario> createScenarios() {
   std::vector<QueryTestScenario> scenarios;
   for (const auto& [lineage, expected] : EXPECTED_ROWS) {
      scenarios.push_back(scenario("EXACT", lineage, "", expected.exact));
      scenarios.push_back(scenario(
         "DO_NOT_FOLLOW",
         lineage,
         ", includeSublineages:=true, recombinantFollowingMode:='doNotFollow'",
         expected.do_not_follow
      ));
      scenarios.push_back(scenario(
         "FOLLOW_IF_FULLY_CONTAINED_IN_CLADE",
         lineage,
         ", includeSublineages:=true, recombinantFollowingMode:='followIfFullyContainedInClade'",
         expected.follow_if_fully_contained_in_clade
      ));
      scenarios.push_back(scenario(
         "ALWAYS_FOLLOW",
         lineage,
         ", includeSublineages:=true, recombinantFollowingMode:='alwaysFollow'",
         expected.always_follow
      ));
   }
   return scenarios;
}

}  // namespace

QUERY_TEST(LineageRecombinantGraphTest, TEST_DATA, ::testing::ValuesIn(createScenarios()))
