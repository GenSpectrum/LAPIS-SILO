#include "silo/query_engine/optimizer/filter_pushdown_pass.h"

#include <map>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/project_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/operators/union_all_node.h"
#include "silo/query_engine/scalar_expressions/literal.h"
#include "silo/query_engine/scalar_expressions/zstd_decompress_scalar.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/table.h"

using silo::query_engine::optimizer::FilterPushdownPass;
namespace operators = silo::query_engine::operators;
namespace scalar_expressions = silo::query_engine::scalar_expressions;

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

std::unique_ptr<scalar_expressions::ScalarExpression> makeDummyFilter() {
   return std::make_unique<scalar_expressions::BoolLiteral>(true);
}

operators::QueryNodePtr makeScan() {
   return std::make_unique<operators::TableScanNode>(
      makeTable(), makeDummyFilter(), std::vector<silo::schema::ColumnIdentifier>{}
   );
}

operators::QueryNodePtr makeFilteredScan(bool filter_value) {
   return std::make_unique<operators::FilterNode>(
      makeScan(), std::make_unique<scalar_expressions::BoolLiteral>(filter_value)
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

TEST(FilterPushdownPass, pushesFilterThroughMapIntoTableScan) {
   auto table = makeTable();
   auto scan = std::make_unique<operators::TableScanNode>(
      table, makeDummyFilter(), std::vector<silo::schema::ColumnIdentifier>{}
   );
   auto filter_node = std::make_unique<operators::FilterNode>(std::move(scan), makeDummyFilter());

   std::vector<operators::MapNode::Assignment> assignments;
   assignments.push_back(
      {.output_column = {.name = "x", .type = silo::schema::ColumnType::INT64},
       .expression = std::make_unique<scalar_expressions::Int64Literal>(3)}
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

// --- FilterNode(MapNode(TableScanNode)): the swap that keeps decompression above the filter ---
//
// This is the motivating case for #1343: a filter stacked on top of a (decompression) MapNode.
// FilterPushdownPass keeps the MapNode on top and pushes the filter down into the TableScan,
// so only the rows matching the filter are ever decompressed.
TEST(FilterPushdownPass, pushesFilterThroughDecompressMapIntoTableScan) {
   using silo::schema::ColumnIdentifier;
   using silo::schema::ColumnType;

   const ColumnIdentifier seq_column{.name = "seq", .type = ColumnType::NUCLEOTIDE_SEQUENCE};
   auto scan = std::make_unique<operators::TableScanNode>(
      makeTable(), makeDummyFilter(), std::vector<ColumnIdentifier>{seq_column}
   );

   std::vector<operators::MapNode::Assignment> assignments;
   assignments.push_back(
      {.output_column = {.name = "seq", .type = ColumnType::STRING},
       .expression = std::make_unique<scalar_expressions::ZstdDecompressScalar>(seq_column, "A")}
   );
   auto map_node = std::make_unique<operators::MapNode>(std::move(scan), std::move(assignments));
   auto filter_node =
      std::make_unique<operators::FilterNode>(std::move(map_node), makeDummyFilter());

   auto result = FilterPushdownPass::run(std::move(filter_node));

   // The FilterNode is gone; the decompression MapNode stays on top with the filter pushed
   // into the TableScan below it.
   ASSERT_EQ(result->kind(), operators::NodeKind::MAP);
   auto* map = dynamic_cast<operators::MapNode*>(result.get());
   ASSERT_EQ(map->assignments.size(), 1);
   EXPECT_EQ(map->assignments.front().expression->kind(), scalar_expressions::ZstdDecompressScalar::KIND);
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
       .expression = std::make_unique<scalar_expressions::Int64Literal>(3)}
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

}  // namespace
