#include "silo/query_engine/optimizer/bitmap_aggregation_rewrite_pass.h"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/scalar_expressions/at.h"
#include "silo/query_engine/scalar_expressions/literal.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_metadata.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/table.h"

using silo::Nucleotide;
using silo::query_engine::optimizer::BitmapAggregationRewritePass;
using silo::schema::ColumnIdentifier;
using silo::schema::ColumnType;
namespace scalar_expressions = silo::query_engine::scalar_expressions;
namespace operators = silo::query_engine::operators;

namespace {

const ColumnIdentifier NUC_COLUMN{.name = "nuc", .type = ColumnType::NUCLEOTIDE_SEQUENCE};
const ColumnIdentifier ID_COLUMN{.name = "id", .type = ColumnType::STRING};
const ColumnIdentifier DIVISION_COLUMN{.name = "division", .type = ColumnType::INDEXED_STRING};

/// A table whose schema carries a nucleotide sequence column "nuc", an indexed string column
/// "division" and the "id" primary key, so the pass can resolve every kind of grouping key against
/// it. The columns hold no data: the pass only reads the schema, it never executes the node.
std::shared_ptr<silo::storage::Table> tableWithColumns() {
   using silo::storage::column::ColumnMetadata;
   using silo::storage::column::IndexedStringColumnMetadata;
   using silo::storage::column::SequenceColumnMetadata;
   using silo::storage::column::StringColumnMetadata;

   std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> col_meta{
      {ID_COLUMN, std::make_shared<StringColumnMetadata>(ID_COLUMN.name)},
      {DIVISION_COLUMN, std::make_shared<IndexedStringColumnMetadata>(DIVISION_COLUMN.name)},
      {NUC_COLUMN,
       std::make_shared<SequenceColumnMetadata<Nucleotide>>(
          NUC_COLUMN.name, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::A}
       )}
   };
   auto schema = std::make_shared<silo::schema::TableSchema>(std::move(col_meta), ID_COLUMN);
   return std::make_shared<silo::storage::Table>(silo::schema::TableName::getDefault(), schema);
}

operators::QueryNodePtr makeScan() {
   return std::make_unique<operators::TableScanNode>(
      tableWithColumns(),
      std::make_unique<scalar_expressions::BoolLiteral>(true),
      std::vector<ColumnIdentifier>{}
   );
}

/// map({<field> := at(<column>, 1)}) over `child`.
operators::QueryNodePtr makeMapWithAt(
   operators::QueryNodePtr child,
   const std::string& field,
   const ColumnIdentifier& at_column
) {
   std::vector<operators::MapNode::Assignment> assignments;
   assignments.push_back(
      {.output_column = {.name = field, .type = ColumnType::STRING},
       .expression = std::make_unique<scalar_expressions::At>(at_column, 1)}
   );
   return std::make_unique<operators::MapNode>(std::move(child), std::move(assignments));
}

/// groupBy({count := count()}, {<fields>}) over `child`, with the single count aggregate optionally
/// carrying a source column (which takes it out of the recognized "bare count()" shape).
operators::QueryNodePtr makeGroupByCount(
   operators::QueryNodePtr child,
   const std::vector<std::string>& fields,
   std::optional<ColumnIdentifier> count_source = std::nullopt
) {
   std::vector<ColumnIdentifier> group_by;
   group_by.reserve(fields.size());
   for (const auto& field : fields) {
      group_by.push_back({.name = field, .type = ColumnType::STRING});
   }
   std::vector<operators::AggregateDefinition> aggregates{
      {.output_name = "count",
       .function = operators::AggregateFunction::COUNT,
       .source_column = std::move(count_source)}
   };
   return std::make_unique<operators::AggregateNode>(
      std::move(child), std::move(group_by), std::move(aggregates)
   );
}

// The canonical mutation co-occurrence shape: groupBy(count) over map(at(sequence column)) over a
// table scan is rewritten into the dedicated BitmapAggregationNode.
TEST(BitmapAggregationRewritePass, rewritesSequencePositionShape) {
   auto node = makeGroupByCount(makeMapWithAt(makeScan(), "s", NUC_COLUMN), {"s"});

   auto result = BitmapAggregationRewritePass::run(std::move(node));

   EXPECT_EQ(result->kind(), operators::NodeKind::BITMAP_AGGREGATION);
}

// groupBy(count) directly on an indexed string column (no map) is rewritten too: the column can be
// grouped straight from its inverted index.
TEST(BitmapAggregationRewritePass, rewritesIndexedColumnShape) {
   auto node = makeGroupByCount(makeScan(), {"division"});

   auto result = BitmapAggregationRewritePass::run(std::move(node));

   EXPECT_EQ(result->kind(), operators::NodeKind::BITMAP_AGGREGATION);
}

// A sequence position and an indexed column can be grouped together in one node.
TEST(BitmapAggregationRewritePass, rewritesMixedShape) {
   auto node = makeGroupByCount(makeMapWithAt(makeScan(), "s", NUC_COLUMN), {"s", "division"});

   auto result = BitmapAggregationRewritePass::run(std::move(node));

   EXPECT_EQ(result->kind(), operators::NodeKind::BITMAP_AGGREGATION);
}

// `at` on a non-sequence column (the STRING primary key) cannot be grouped by the bitmap engine, so
// the shape is not recognized and the AggregateNode is left in place.
TEST(BitmapAggregationRewritePass, declinesWhenAtReadsNonSequenceColumn) {
   auto node = makeGroupByCount(makeMapWithAt(makeScan(), "s", ID_COLUMN), {"s"});

   auto result = BitmapAggregationRewritePass::run(std::move(node));

   EXPECT_EQ(result->kind(), operators::NodeKind::AGGREGATE);
}

// Grouping directly on a non-indexed string column (the primary key) has no inverted index to read,
// so the pass declines and the generic pipeline handles it.
TEST(BitmapAggregationRewritePass, declinesWhenGroupingOnNonIndexedColumn) {
   auto node = makeGroupByCount(makeScan(), {"id"});

   auto result = BitmapAggregationRewritePass::run(std::move(node));

   EXPECT_EQ(result->kind(), operators::NodeKind::AGGREGATE);
}

// A count with a source column is not the bare count() the rewrite recognizes, so it declines.
TEST(BitmapAggregationRewritePass, declinesWhenAggregateIsNotBareCount) {
   auto node = makeGroupByCount(makeMapWithAt(makeScan(), "s", NUC_COLUMN), {"s"}, NUC_COLUMN);

   auto result = BitmapAggregationRewritePass::run(std::move(node));

   EXPECT_EQ(result->kind(), operators::NodeKind::AGGREGATE);
}

// When the matched pipeline does not sit directly on a table scan (here a FilterNode is in
// between), the pass must decline gracefully rather than throw: an optimizer may never turn a valid
// query into an error. The AggregateNode is left untouched for the generic pipeline to execute.
TEST(BitmapAggregationRewritePass, declinesWithoutThrowingWhenChildIsNotScan) {
   auto filtered_scan = std::make_unique<operators::FilterNode>(
      makeScan(), std::make_unique<scalar_expressions::BoolLiteral>(true)
   );
   auto node = makeGroupByCount(makeMapWithAt(std::move(filtered_scan), "s", NUC_COLUMN), {"s"});

   operators::QueryNodePtr result;
   ASSERT_NO_THROW(result = BitmapAggregationRewritePass::run(std::move(node)));
   EXPECT_EQ(result->kind(), operators::NodeKind::AGGREGATE);
}

}  // namespace
