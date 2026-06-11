#include "silo/query_engine/filter_pushdown_pass.h"

#include <map>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/query_engine/expressions/literal.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/table.h"

using silo::query_engine::FilterPushdownPass;
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
   EXPECT_EQ(table_scan->filter->toString(), "And(And(true & true))");
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
   EXPECT_EQ(table_scan->filter->toString(), "And(And(true & true))");
}

}  // namespace
