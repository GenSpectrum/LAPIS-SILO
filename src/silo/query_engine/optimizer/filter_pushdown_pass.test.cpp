#include "silo/query_engine/optimizer/filter_pushdown_pass.h"

#include <map>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/query_engine/expressions/literal.h"
#include "silo/query_engine/expressions/string_equals.h"
#include "silo/query_engine/expressions/zstd_decompress_scalar.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/project_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/operators/union_all_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/table.h"

using silo::query_engine::optimizer::FilterPushdownPass;
namespace operators = silo::query_engine::operators;
namespace expressions = silo::query_engine::expressions;

namespace {

std::shared_ptr<silo::storage::Table> makeTable() {
   using silo::schema::ColumnIdentifier;
   using silo::schema::ColumnType;
   using silo::storage::column::ColumnMetadata;
   using silo::storage::column::StringColumnMetadata;

   ColumnIdentifier primary_key{.name = "id", .type = ColumnType::STRING};
   std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> col_meta{
      {primary_key, std::make_shared<StringColumnMetadata>(primary_key.name)}
   };
   auto schema = std::make_shared<silo::schema::TableSchema>(std::move(col_meta), primary_key);
   return std::make_shared<silo::storage::Table>(silo::schema::TableName("default"), schema);
}

std::unique_ptr<expressions::Expression> makeDummyFilter() {
   return std::make_unique<expressions::BoolLiteral>(true);
}

operators::QueryNodePtr makeScan() {
   return std::make_unique<operators::TableScanNode>(
      makeTable(), makeDummyFilter(), std::vector<silo::schema::ColumnIdentifier>{}
   );
}

operators::QueryNodePtr makeFilteredScan(bool filter_value) {
   return std::make_unique<operators::FilterNode>(
      makeScan(), std::make_unique<expressions::BoolLiteral>(filter_value)
   );
}

// --- FilterNode(TableScanNode) ---

TEST(FilterPushdownPass, eliminatesFilterNodeAboveTableScan) {
   auto table = makeTable();
   auto scan = std::make_unique<operators::TableScanNode>(
      table, makeDummyFilter(), std::vector<silo::schema::ColumnIdentifier>{}
   );
   auto filter_node = std::make_unique<operators::FilterNode>(std::move(scan), makeDummyFilter());

   auto result = FilterPushdownPass::run(std::move(filter_node));

   // The FilterNode is gone; result is the TableScanNode directly.
   EXPECT_EQ(result->kind(), operators::NodeKind::TABLE_SCAN);
   auto* table_scan = dynamic_cast<operators::TableScanNode*>(result.get());
   EXPECT_EQ(table_scan->filter->toString(), "And(true & true)");
}

// --- FilterNode(FilterNode(TableScanNode)) ---
TEST(FilterPushdownPass, eliminatesStackedFilterNodesAboveTableScan) {
   auto inner_filter = makeFilteredScan(false);
   auto outer_filter =
      std::make_unique<operators::FilterNode>(std::move(inner_filter), makeDummyFilter());

   auto result = FilterPushdownPass::run(std::move(outer_filter));

   // Both FilterNodes are gone; result is the TableScanNode with all three filters merged.
   EXPECT_EQ(result->kind(), operators::NodeKind::TABLE_SCAN);
   auto* table_scan = dynamic_cast<operators::TableScanNode*>(result.get());
   EXPECT_EQ(table_scan->filter->toString(), "And(true & false & true)");
}

// --- MapNode(FilterNode(TableScanNode)) ---

// The filter references no column the MapNode produces (the Map adds an independent
// column `x := 3`), so the filter is independent and is pushed through the Map into the
// TableScan.
TEST(FilterPushdownPass, pushesFilterThroughMapIntoTableScan) {
   auto table = makeTable();
   auto scan = std::make_unique<operators::TableScanNode>(
      table, makeDummyFilter(), std::vector<silo::schema::ColumnIdentifier>{}
   );
   auto filter_node = std::make_unique<operators::FilterNode>(std::move(scan), makeDummyFilter());

   std::vector<operators::MapNode::Assignment> assignments;
   assignments.push_back(
      {.output_column = {.name = "x", .type = silo::schema::ColumnType::INT64},
       .expression = std::make_unique<expressions::Int64Literal>(3)}
   );
   auto map_node =
      std::make_unique<operators::MapNode>(std::move(filter_node), std::move(assignments));

   auto result = FilterPushdownPass::run(std::move(map_node));

   // The MapNode is retained; the FilterNode below it was pushed into the TableScan.
   ASSERT_EQ(result->kind(), operators::NodeKind::MAP);
   auto* map = dynamic_cast<operators::MapNode*>(result.get());
   ASSERT_EQ(map->child->kind(), operators::NodeKind::TABLE_SCAN);
   auto* table_scan = dynamic_cast<operators::TableScanNode*>(map->child.get());
   EXPECT_EQ(table_scan->filter->toString(), "And(true & true)");
}

// --- FilterNode(ProjectNode(MapNode(FilterNode(TableScanNode)))) ---
TEST(FilterPushdownPass, pushesFilterThroughProjectAndMapIntoTableScan) {
   auto inner_filter = makeFilteredScan(false);

   std::vector<operators::MapNode::Assignment> assignments;
   assignments.push_back(
      {.output_column = {.name = "x", .type = silo::schema::ColumnType::INT64},
       .expression = std::make_unique<expressions::Int64Literal>(3)}
   );
   auto map_node =
      std::make_unique<operators::MapNode>(std::move(inner_filter), std::move(assignments));
   auto project_node = std::make_unique<operators::ProjectNode>(
      std::move(map_node),
      std::vector<silo::schema::ColumnIdentifier>{
         {.name = "x", .type = silo::schema::ColumnType::INT64}
      }
   );
   auto outer_filter =
      std::make_unique<operators::FilterNode>(std::move(project_node), makeDummyFilter());

   auto result = FilterPushdownPass::run(std::move(outer_filter));

   ASSERT_EQ(result->kind(), operators::NodeKind::PROJECT);
   auto* project = dynamic_cast<operators::ProjectNode*>(result.get());
   ASSERT_EQ(project->child->kind(), operators::NodeKind::MAP);
   auto* map = dynamic_cast<operators::MapNode*>(project->child.get());
   ASSERT_EQ(map->child->kind(), operators::NodeKind::TABLE_SCAN);
   auto* table_scan = dynamic_cast<operators::TableScanNode*>(map->child.get());
   EXPECT_EQ(table_scan->filter->toString(), "And(true & false & true)");
}

// --- FilterNode(UnionAllNode(FilterNode(TableScanNode), FilterNode(TableScanNode))) ---
TEST(FilterPushdownPass, pushesFilterIntoBothUnionAllBranches) {
   // left branch: Filter(false, Scan), right branch: Filter(true, Scan)
   auto union_all =
      std::make_unique<operators::UnionAllNode>(makeFilteredScan(false), makeFilteredScan(true));
   auto filter_node =
      std::make_unique<operators::FilterNode>(std::move(union_all), makeDummyFilter());

   auto result = FilterPushdownPass::run(std::move(filter_node));

   ASSERT_EQ(result->kind(), operators::NodeKind::UNION_ALL);
   auto* union_node = dynamic_cast<operators::UnionAllNode*>(result.get());
   ASSERT_EQ(union_node->left->kind(), operators::NodeKind::TABLE_SCAN);
   ASSERT_EQ(union_node->right->kind(), operators::NodeKind::TABLE_SCAN);
   auto* left_scan = dynamic_cast<operators::TableScanNode*>(union_node->left.get());
   auto* right_scan = dynamic_cast<operators::TableScanNode*>(union_node->right.get());
   // outer filter (true) + branch filter (false/true) + scan filter (true)
   EXPECT_EQ(left_scan->filter->toString(), "And(true & false & true)");
   EXPECT_EQ(right_scan->filter->toString(), "And(true & true & true)");
}

// --- Filter(predicate on Map-produced col) → Map(produces col) → Scan ---
//
// The filter predicate references `seq`, which is the OUTPUT column of the MapNode
// (the MapNode produces `seq` by decompressing the same-named compressed column).
// FilterPushdownPass must NOT push this predicate below the Map: doing so would move
// the predicate to the TableScan where `seq` is still the raw compressed column, making
// the filter semantically wrong.
TEST(FilterPushdownPass, doesNotPushFilterThroughMapWhenFilterReferencesProducedColumn) {
   using silo::schema::ColumnIdentifier;
   using silo::schema::ColumnType;

   // Map produces `seq` (STRING) by decompressing the compressed `seq` column.
   const ColumnIdentifier compressed_col{.name = "seq", .type = ColumnType::ZSTD_COMPRESSED_STRING};
   std::vector<operators::MapNode::Assignment> assignments;
   assignments.push_back(
      {.output_column = {.name = "seq", .type = ColumnType::STRING},
       .expression =
          std::make_unique<expressions::ZstdDecompressScalar>(compressed_col, "dictionary")}
   );
   auto map_node = std::make_unique<operators::MapNode>(makeScan(), std::move(assignments));

   // Filter references `seq` — the Map-produced column (not the compressed source).
   auto filter_node = std::make_unique<operators::FilterNode>(
      std::move(map_node),
      std::make_unique<expressions::StringEquals>("seq", std::optional<std::string>{"ACGT"})
   );

   auto result = FilterPushdownPass::run(std::move(filter_node));

   // The Map-produced column `seq` is referenced by the filter predicate (a StringEquals on
   // the decompressed STRING), so the filter must NOT be pushed through the MapNode. The
   // FilterNode must survive above the Map.
   ASSERT_EQ(result->kind(), operators::NodeKind::FILTER)
      << "FilterPushdownPass incorrectly pushed a filter referencing a Map-produced column "
         "below the MapNode (bug: filter would run against the compressed column, not the "
         "decompressed one)";
   // Additionally: the scan must NOT have `seq = 'ACGT'` in its filter (it must not be
   // pushed past the MapNode).
   const auto* filter_result = dynamic_cast<operators::FilterNode*>(result.get());
   ASSERT_NE(filter_result, nullptr);
   ASSERT_EQ(filter_result->child->kind(), operators::NodeKind::MAP);
   const auto* map = dynamic_cast<operators::MapNode*>(filter_result->child.get());
   ASSERT_NE(map, nullptr);
   ASSERT_EQ(map->child->kind(), operators::NodeKind::TABLE_SCAN);
   const auto* scan = dynamic_cast<operators::TableScanNode*>(map->child.get());
   ASSERT_NE(scan, nullptr);
   EXPECT_EQ(scan->filter->toString(), "And(true)")
      << "seq='ACGT' was incorrectly pushed into the TableScan past the MapNode";
}

// Helper: a MapNode that decompresses `seq` (sequence → STRING, same name).
operators::QueryNodePtr makeDecompressMapOverScan() {
   using silo::schema::ColumnIdentifier;
   using silo::schema::ColumnType;
   const ColumnIdentifier compressed_col{.name = "seq", .type = ColumnType::ZSTD_COMPRESSED_STRING};
   std::vector<operators::MapNode::Assignment> assignments;
   assignments.push_back(
      {.output_column = {.name = "seq", .type = ColumnType::STRING},
       .expression =
          std::make_unique<expressions::ZstdDecompressScalar>(compressed_col, "dictionary")}
   );
   return std::make_unique<operators::MapNode>(makeScan(), std::move(assignments));
}

// Two stacked filters above a decompression Map: one references the produced column `seq`
// (dependent → must stay above the Map), the other references an unrelated column (`country`,
// independent → must be pushed through into the scan). The pass must SPLIT them.
TEST(FilterPushdownPass, splitsFiltersKeepingProducedColumnFilterAboveMap) {
   auto map_node = makeDecompressMapOverScan();
   auto inner_filter = std::make_unique<operators::FilterNode>(
      std::move(map_node),
      std::make_unique<expressions::StringEquals>("country", std::optional<std::string>{"CH"})
   );
   auto outer_filter = std::make_unique<operators::FilterNode>(
      std::move(inner_filter),
      std::make_unique<expressions::StringEquals>("seq", std::optional<std::string>{"ACGT"})
   );

   auto result = FilterPushdownPass::run(std::move(outer_filter));

   // Result: Filter(seq='ACGT') → Map(seq) → Scan{filter includes country='CH'}.
   ASSERT_EQ(result->kind(), operators::NodeKind::FILTER);
   const auto* filter = dynamic_cast<operators::FilterNode*>(result.get());
   ASSERT_NE(filter, nullptr);
   EXPECT_EQ(filter->filter->toString(), "seq = 'ACGT'");
   ASSERT_EQ(filter->child->kind(), operators::NodeKind::MAP);
   const auto* map = dynamic_cast<operators::MapNode*>(filter->child.get());
   ASSERT_NE(map, nullptr);
   ASSERT_EQ(map->child->kind(), operators::NodeKind::TABLE_SCAN);
   const auto* scan = dynamic_cast<operators::TableScanNode*>(map->child.get());
   ASSERT_NE(scan, nullptr);
   EXPECT_EQ(scan->filter->toString(), "And(country = 'CH' & true)");
}

TEST(FilterPushdownPass, mergesMultipleDependentFiltersAboveMap) {
   auto map_node = makeDecompressMapOverScan();

   auto inner_filter = std::make_unique<operators::FilterNode>(
      std::move(map_node),
      std::make_unique<expressions::StringEquals>("seq", std::optional<std::string>{"ACGT"})
   );
   auto outer_filter = std::make_unique<operators::FilterNode>(
      std::move(inner_filter),
      std::make_unique<expressions::StringEquals>("seq", std::optional<std::string>{"TTTT"})
   );

   auto result = FilterPushdownPass::run(std::move(outer_filter));

   ASSERT_EQ(result->kind(), operators::NodeKind::FILTER);
   const auto* filter = dynamic_cast<operators::FilterNode*>(result.get());
   ASSERT_NE(filter, nullptr);
   EXPECT_EQ(filter->filter->toString(), "And(seq = 'TTTT' & seq = 'ACGT')");
   ASSERT_EQ(filter->child->kind(), operators::NodeKind::MAP);
   const auto* map = dynamic_cast<operators::MapNode*>(filter->child.get());
   ASSERT_NE(map, nullptr);
   const auto* scan = dynamic_cast<operators::TableScanNode*>(map->child.get());
   ASSERT_NE(scan, nullptr);
   EXPECT_EQ(scan->filter->toString(), "And(true)");
}

}  // namespace
