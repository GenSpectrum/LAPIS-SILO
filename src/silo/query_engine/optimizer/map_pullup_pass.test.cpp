#include "silo/query_engine/optimizer/map_pullup_pass.h"

#include <map>
#include <memory>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "silo/query_engine/expressions/field_ref.h"
#include "silo/query_engine/expressions/literal.h"
#include "silo/query_engine/expressions/zstd_decompress_scalar.h"
#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/table.h"

using silo::query_engine::optimizer::MapPullupPass;
namespace expressions = silo::query_engine::expressions;
namespace operators = silo::query_engine::operators;
using silo::schema::ColumnIdentifier;
using silo::schema::ColumnType;

namespace {

std::shared_ptr<silo::storage::Table> makeTable() {
   using silo::storage::column::ColumnMetadata;
   using silo::storage::column::StringColumnMetadata;

   ColumnIdentifier primary_key{.name = "id", .type = ColumnType::STRING};
   std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> col_meta{
      {primary_key, std::make_shared<StringColumnMetadata>(primary_key.name)}
   };
   auto schema = std::make_shared<silo::schema::TableSchema>(std::move(col_meta), primary_key);
   return std::make_shared<silo::storage::Table>(silo::schema::TableName::getDefault(), schema);
}

operators::QueryNodePtr makeScan(std::vector<ColumnIdentifier> fields) {
   return std::make_unique<operators::TableScanNode>(
      makeTable(), std::make_unique<expressions::BoolLiteral>(true), std::move(fields)
   );
}

TEST(MapPullupPass, pullsMapAboveFetch) {
   auto seq_column = ColumnIdentifier{.name = "seq", .type = ColumnType::NUCLEOTIDE_SEQUENCE};
   std::vector<operators::MapNode::Assignment> assignments;
   assignments.push_back(
      {.output_column = {.name = "seq", .type = ColumnType::STRING},
       .expression = std::make_unique<expressions::ZstdDecompressScalar>(seq_column, "A")}
   );
   auto map_node =
      std::make_unique<operators::MapNode>(makeScan({seq_column}), std::move(assignments));
   auto root = std::make_unique<operators::FetchNode>(std::move(map_node), 10U, 5U);

   auto result = MapPullupPass::run(std::move(root));

   ASSERT_EQ(result->kind(), operators::NodeKind::MAP);
   auto* map = dynamic_cast<operators::MapNode*>(result.get());
   ASSERT_EQ(map->child->kind(), operators::NodeKind::FETCH);
   auto* fetch = dynamic_cast<operators::FetchNode*>(map->child.get());
   EXPECT_EQ(fetch->count, 10U);
   EXPECT_EQ(fetch->offset, 5U);
}

TEST(MapPullupPass, bubblesMapThroughStackedFetchNodes) {
   std::vector<operators::MapNode::Assignment> assignments;
   assignments.push_back(
      {.output_column = {.name = "x", .type = ColumnType::INT64},
       .expression = std::make_unique<expressions::Int64Literal>(3)}
   );
   auto map_node = std::make_unique<operators::MapNode>(makeScan({}), std::move(assignments));
   auto lower_fetch = std::make_unique<operators::FetchNode>(std::move(map_node), 5U, std::nullopt);
   auto root = std::make_unique<operators::FetchNode>(std::move(lower_fetch), 2U, std::nullopt);

   auto result = MapPullupPass::run(std::move(root));

   ASSERT_EQ(result->kind(), operators::NodeKind::MAP);
   auto* map = dynamic_cast<operators::MapNode*>(result.get());
   ASSERT_EQ(map->child->kind(), operators::NodeKind::FETCH);
   auto* outer_fetch = dynamic_cast<operators::FetchNode*>(map->child.get());
   ASSERT_EQ(outer_fetch->child->kind(), operators::NodeKind::FETCH);
}

TEST(MapPullupPass, pullsMapAboveFilterWhenFilterDoesNotUseProducedColumns) {
   std::vector<operators::MapNode::Assignment> assignments;
   assignments.push_back(
      {.output_column = {.name = "x", .type = ColumnType::INT64},
       .expression = std::make_unique<expressions::Int64Literal>(3)}
   );
   auto map_node = std::make_unique<operators::MapNode>(makeScan({}), std::move(assignments));
   auto root = std::make_unique<operators::FilterNode>(
      std::move(map_node), std::make_unique<expressions::BoolLiteral>(true)
   );

   auto result = MapPullupPass::run(std::move(root));

   ASSERT_EQ(result->kind(), operators::NodeKind::MAP);
   auto* map = dynamic_cast<operators::MapNode*>(result.get());
   ASSERT_EQ(map->child->kind(), operators::NodeKind::FILTER);
}

TEST(MapPullupPass, keepsMapBelowFilterWhenFilterUsesProducedColumns) {
   std::vector<operators::MapNode::Assignment> assignments;
   assignments.push_back(
      {.output_column = {.name = "x", .type = ColumnType::INT64},
       .expression = std::make_unique<expressions::Int64Literal>(3)}
   );
   auto map_node = std::make_unique<operators::MapNode>(makeScan({}), std::move(assignments));
   auto root = std::make_unique<operators::FilterNode>(
      std::move(map_node),
      std::make_unique<expressions::FieldRef>(
         ColumnIdentifier{.name = "x", .type = ColumnType::INT64}
      )
   );

   auto result = MapPullupPass::run(std::move(root));

   ASSERT_EQ(result->kind(), operators::NodeKind::FILTER);
   auto* filter = dynamic_cast<operators::FilterNode*>(result.get());
   ASSERT_EQ(filter->child->kind(), operators::NodeKind::MAP);
}

}  // namespace
