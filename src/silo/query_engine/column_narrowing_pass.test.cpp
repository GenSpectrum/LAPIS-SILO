#include "silo/query_engine/column_narrowing_pass.h"

#include <memory>
#include <optional>
#include <vector>

#include "silo/query_engine/operator_visitor.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/query_engine/filter/expressions/true.h"
#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/order_by_node.h"
#include "silo/query_engine/operators/project_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/order_by_field.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/table.h"

using silo::schema::ColumnIdentifier;
using silo::schema::ColumnType;
namespace operators = silo::query_engine::operators;

namespace {

ColumnIdentifier col(std::string name) {
   return ColumnIdentifier{.name = std::move(name), .type = ColumnType::STRING};
}

std::shared_ptr<silo::storage::Table> dummyTable() {
   using silo::storage::column::ColumnMetadata;
   using silo::storage::column::StringColumnMetadata;

   ColumnIdentifier primary_key = col("id");
   std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> col_meta{
      {primary_key, std::make_shared<StringColumnMetadata>(primary_key.name)}
   };
   auto schema = std::make_shared<silo::schema::TableSchema>(std::move(col_meta), primary_key);
   return std::make_shared<silo::storage::Table>(schema);
}

// The narrowing pass only inspects the scan's field list, so the table identity is irrelevant.
std::unique_ptr<operators::TableScanNode> makeScan(std::vector<ColumnIdentifier> fields) {
   return std::make_unique<operators::TableScanNode>(
      dummyTable(),
      std::make_unique<silo::query_engine::filter::expressions::True>(),
      std::move(fields)
   );
}

std::vector<ColumnIdentifier> scanSchema(const operators::TableScanNode& node) {
   return node.fields;
}

// Extracts the TableScanNode from the bottom of a chain of unary nodes.
// NOLINTNEXTLINE(misc-no-recursion)
operators::TableScanNode& leafScan(operators::QueryNode& node) {
   if (auto* scan = dynamic_cast<operators::TableScanNode*>(&node)) {
      return *scan;
   }
   if (auto* proj = dynamic_cast<operators::ProjectNode*>(&node)) {
      return leafScan(*proj->child);
   }
   if (auto* fetch = dynamic_cast<operators::FetchNode*>(&node)) {
      return leafScan(*fetch->child);
   }
   if (auto* order = dynamic_cast<operators::OrderByNode*>(&node)) {
      return leafScan(*order->child);
   }
   if (auto* agg = dynamic_cast<operators::AggregateNode*>(&node)) {
      return leafScan(*agg->child);
   }
   if (auto* flt = dynamic_cast<operators::FilterNode*>(&node)) {
      return leafScan(*flt->child);
   }
   throw std::logic_error("unexpected node type in leafScan");
}

// --- TableScanNode ---

TEST(ColumnNarrowingPassScan, keepsOnlyRequiredColumns) {
   auto scan = makeScan({col("a"), col("b"), col("c")});
   silo::query_engine::ColumnNarrowingPass pass({col("b")});
   pass(*scan);

   EXPECT_THAT(scanSchema(*scan), ::testing::ElementsAre(col("b")));
}

TEST(ColumnNarrowingPassScan, removesAllWhenNoneRequired) {
   auto scan = makeScan({col("a"), col("b")});
   silo::query_engine::ColumnNarrowingPass pass({});
   pass(*scan);

   EXPECT_THAT(scanSchema(*scan), ::testing::IsEmpty());
}

TEST(ColumnNarrowingPassScan, keepsAllWhenAllRequired) {
   auto scan = makeScan({col("x"), col("y")});
   silo::query_engine::ColumnNarrowingPass pass({col("x"), col("y")});
   pass(*scan);

   EXPECT_THAT(scanSchema(*scan), ::testing::ElementsAre(col("x"), col("y")));
}

// --- ProjectNode -> TableScanNode ---

TEST(ColumnNarrowingPassProject, narrowsScanToProjectedColumns) {
   auto scan = makeScan({col("a"), col("b"), col("c")});
   operators::QueryNodePtr node =
      std::make_unique<operators::ProjectNode>(std::move(scan), std::vector{col("a")});

   silo::query_engine::ColumnNarrowingPass pass({col("a"), col("b"), col("c")});
   if (auto new_node = operators::visit(*node, pass)) {
      node = std::move(new_node);
   }

   EXPECT_THAT(scanSchema(leafScan(*node)), ::testing::ElementsAre(col("a")));
}

// --- AggregateNode -> TableScanNode ---

TEST(ColumnNarrowingPassAggregate, narrowsScanToGroupByColumns) {
   auto scan = makeScan({col("a"), col("b"), col("c")});
   auto agg = std::make_unique<operators::AggregateNode>(
      std::move(scan),
      std::vector{col("b")},
      std::vector<operators::AggregateDefinition>{
         {.output_name = "cnt",
          .function = operators::AggregateFunction::COUNT,
          .source_column = std::nullopt}
      }
   );

   silo::query_engine::ColumnNarrowingPass pass({col("a"), col("b"), col("c")});
   pass(*agg);

   EXPECT_THAT(scanSchema(leafScan(*agg)), ::testing::ElementsAre(col("b")));
}

TEST(ColumnNarrowingPassAggregate, countStarWithNoGroupByKeepsOneColumn) {
   auto scan = makeScan({col("a"), col("b"), col("c")});
   auto agg = std::make_unique<operators::AggregateNode>(
      std::move(scan),
      std::vector<ColumnIdentifier>{},
      std::vector<operators::AggregateDefinition>{
         {.output_name = "cnt",
          .function = operators::AggregateFunction::COUNT,
          .source_column = std::nullopt}
      }
   );

   silo::query_engine::ColumnNarrowingPass pass({col("a"), col("b"), col("c")});
   pass(*agg);

   // COUNT(*) with no group-by needs only one column to drive the row stream.
   EXPECT_THAT(scanSchema(leafScan(*agg)), ::testing::SizeIs(1));
}

// --- OrderByNode -> TableScanNode ---

TEST(ColumnNarrowingPassOrderBy, appendsSortKeyToRequired) {
   auto scan = makeScan({col("a"), col("b"), col("c")});
   auto order = std::make_unique<operators::OrderByNode>(
      std::move(scan),
      std::vector<silo::query_engine::OrderByField>{{.field = col("c"), .ascending = true}},
      std::nullopt
   );

   // Only "a" is required from above; the pass must add "c" for sorting.
   silo::query_engine::ColumnNarrowingPass pass({col("a")});
   pass(*order);

   EXPECT_THAT(scanSchema(leafScan(*order)), ::testing::UnorderedElementsAre(col("a"), col("c")));
}

// --- ProjectNode elimination ---

TEST(ColumnNarrowingPassProject, collapsesWhenScanNarrowedToProjectFields) {
   auto scan = makeScan({col("a"), col("b"), col("c")});
   auto proj =
      std::make_unique<operators::ProjectNode>(std::move(scan), std::vector{col("a"), col("b")});
   auto fetch = std::make_unique<operators::FetchNode>(std::move(proj), std::nullopt, std::nullopt);

   silo::query_engine::ColumnNarrowingPass pass({col("a"), col("b")});
   pass(*fetch);

   // The ProjectNode should be removed; FetchNode's child is now directly the TableScanNode.
   EXPECT_NE(dynamic_cast<operators::TableScanNode*>(fetch->child.get()), nullptr);
   EXPECT_THAT(scanSchema(leafScan(*fetch)), ::testing::ElementsAre(col("a"), col("b")));
}

TEST(ColumnNarrowingPassProject, stackedProjectsDoNotPropagateSuperfluousColumnsToScan) {
   // Only "a" is needed from outside, but the outer project exposes [a, b] and the inner [a, b, c].
   // The scan should still be narrowed to just [a].
   auto scan = makeScan({col("a"), col("b"), col("c"), col("d")});
   auto inner_proj = std::make_unique<operators::ProjectNode>(
      std::move(scan), std::vector{col("a"), col("b"), col("c")}
   );
   operators::QueryNodePtr outer_proj = std::make_unique<operators::ProjectNode>(
      std::move(inner_proj), std::vector{col("a"), col("b")}
   );

   silo::query_engine::ColumnNarrowingPass pass({col("a")});
   if (auto new_node = operators::visit(*outer_proj, pass)) {
      outer_proj = std::move(new_node);
   }

   EXPECT_THAT(scanSchema(leafScan(*outer_proj)), ::testing::ElementsAre(col("a")));
}

TEST(ColumnNarrowingPassProject, stackedProjectsAreCollapsedAfterNarrowing) {
   auto scan = makeScan({col("a"), col("b"), col("c"), col("d")});
   auto inner_proj = std::make_unique<operators::ProjectNode>(
      std::move(scan), std::vector{col("a"), col("b"), col("c")}
   );
   auto outer_proj = std::make_unique<operators::ProjectNode>(
      std::move(inner_proj), std::vector{col("a"), col("b")}
   );
   auto fetch =
      std::make_unique<operators::FetchNode>(std::move(outer_proj), std::nullopt, std::nullopt);

   silo::query_engine::ColumnNarrowingPass pass({col("a")});
   pass(*fetch);

   EXPECT_NE(dynamic_cast<operators::TableScanNode*>(fetch->child.get()), nullptr);
   EXPECT_THAT(scanSchema(leafScan(*fetch)), ::testing::ElementsAre(col("a")));
}

// --- FetchNode -> TableScanNode ---

TEST(ColumnNarrowingPassFetch, propagatesRequiredThroughFetch) {
   auto scan = makeScan({col("a"), col("b"), col("c")});
   auto fetch = std::make_unique<operators::FetchNode>(std::move(scan), std::nullopt, std::nullopt);

   silo::query_engine::ColumnNarrowingPass pass({col("b")});
   pass(*fetch);

   EXPECT_THAT(scanSchema(leafScan(*fetch)), ::testing::ElementsAre(col("b")));
}

}  // namespace
