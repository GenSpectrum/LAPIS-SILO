#include "silo/query_engine/node_resolution_pass.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/expressions/literal.h"
#include "silo/query_engine/expressions/true.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/operators/unresolved_insertions_node.h"
#include "silo/query_engine/operators/unresolved_most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/unresolved_mutations_node.h"
#include "silo/query_engine/operators/unresolved_phylo_subtree_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/table.h"

using silo::AminoAcid;
using silo::Nucleotide;
using silo::query_engine::IllegalQueryException;
using silo::query_engine::NodeResolutionPass;
namespace operators = silo::query_engine::operators;

namespace {

std::map<silo::schema::TableName, std::shared_ptr<silo::storage::Table>> makeTablesWithDefault() {
   using silo::schema::ColumnIdentifier;
   using silo::schema::ColumnType;
   using silo::storage::column::ColumnMetadata;
   using silo::storage::column::StringColumnMetadata;

   ColumnIdentifier primary_key{.name = "id", .type = ColumnType::STRING};
   std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> col_meta{
      {primary_key, std::make_shared<StringColumnMetadata>(primary_key.name)}
   };
   auto schema = std::make_shared<silo::schema::TableSchema>(std::move(col_meta), primary_key);
   std::map<silo::schema::TableName, std::shared_ptr<silo::storage::Table>> tables;
   tables[silo::schema::TableName::getDefault()] =
      std::make_shared<silo::storage::Table>(silo::schema::TableName::getDefault(), schema);
   return tables;
}

operators::QueryNodePtr makeTableScan() {
   auto tables = makeTablesWithDefault();
   return std::make_unique<operators::TableScanNode>(
      tables.at(silo::schema::TableName{"default"}),
      std::make_unique<silo::query_engine::expressions::True>(),
      std::vector<silo::schema::ColumnIdentifier>{}
   );
}

operators::QueryNodePtr makeNonScanChild() {
   // A FilterNode is not a TableScanNode — used to exercise the "must be a table scan" error path.
   return std::make_unique<operators::FilterNode>(
      makeTableScan(), std::make_unique<silo::query_engine::expressions::True>()
   );
}

std::vector<operators::MapNode::Assignment> makeMapAssignments() {
   std::vector<operators::MapNode::Assignment> assignments;
   assignments.push_back(
      {.output_column = {.name = "x", .type = silo::schema::ColumnType::INT64},
       .expression = std::make_unique<silo::query_engine::expressions::Int64Literal>(3)}
   );
   return assignments;
}

// --- mutations() ---

TEST(NodeResolutionPassMutations, onNonScanNodeThrows) {
   auto node = std::make_unique<operators::UnresolvedMutationsNode<Nucleotide>>(
      makeNonScanChild(), std::vector<std::string>{}, 0.0, std::vector<std::string>{}
   );
   EXPECT_THAT(
      [&]() { (void)NodeResolutionPass::run(std::move(node)); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("mutations() must be applied to a table scan")
      )
   );
}

TEST(NodeResolutionPassMutations, invalidAASequenceNameThrows) {
   std::vector<std::string> seq_names{"noseq"};
   auto node = std::make_unique<operators::UnresolvedMutationsNode<AminoAcid>>(
      makeTableScan(), std::move(seq_names), 0.0, std::vector<std::string>{}
   );
   EXPECT_THAT(
      [&]() { (void)NodeResolutionPass::run(std::move(node)); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("The database does not contain the AminoAcid sequence 'noseq'")
      )
   );
}

TEST(NodeResolutionPassMutations, invalidFieldThrows) {
   std::vector<std::string> bad_fields{"badfield"};
   auto node = std::make_unique<operators::UnresolvedMutationsNode<Nucleotide>>(
      makeTableScan(), std::vector<std::string>{}, 0.0, std::move(bad_fields)
   );
   EXPECT_THAT(
      [&]() { (void)NodeResolutionPass::run(std::move(node)); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("The attribute 'fields' contains an invalid field 'badfield'")
      )
   );
}

// --- insertions() ---

TEST(NodeResolutionPassInsertions, onNonScanNodeThrows) {
   auto node = std::make_unique<operators::UnresolvedInsertionsNode<Nucleotide>>(
      makeNonScanChild(), std::vector<std::string>{}
   );
   EXPECT_THAT(
      [&]() { (void)NodeResolutionPass::run(std::move(node)); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("insertions() must be applied to a table scan")
      )
   );
}

TEST(NodeResolutionPassInsertions, invalidAASequenceNameThrows) {
   std::vector<std::string> seq_names{"noseq"};
   auto node = std::make_unique<operators::UnresolvedInsertionsNode<AminoAcid>>(
      makeTableScan(), std::move(seq_names)
   );
   EXPECT_THAT(
      [&]() { (void)NodeResolutionPass::run(std::move(node)); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("The database does not contain the AminoAcid sequence 'noseq'")
      )
   );
}

// --- phyloSubtree() ---

TEST(NodeResolutionPassPhyloSubtree, onNonScanNodeThrows) {
   auto node = std::make_unique<operators::UnresolvedPhyloSubtreeNode>(
      makeNonScanChild(), std::string{"col"}, false, false
   );
   EXPECT_THAT(
      [&]() { (void)NodeResolutionPass::run(std::move(node)); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("phyloSubtree() must be applied to a table scan")
      )
   );
}

// --- mostRecentCommonAncestor() ---

TEST(NodeResolutionPassMostRecentCommonAncestor, onNonScanNodeThrows) {
   auto node = std::make_unique<operators::UnresolvedMostRecentCommonAncestorNode>(
      makeNonScanChild(), std::string{"col"}, false
   );
   EXPECT_THAT(
      [&]() { (void)NodeResolutionPass::run(std::move(node)); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("mostRecentCommonAncestor() must be applied to a table scan")
      )
   );
}

// --- map() ---

TEST(NodeResolutionPassMap, resolvesUnresolvedChildBelowMap) {
   auto unresolved = std::make_unique<operators::UnresolvedInsertionsNode<Nucleotide>>(
      makeTableScan(), std::vector<std::string>{}
   );
   auto map = std::make_unique<operators::MapNode>(std::move(unresolved), makeMapAssignments());

   auto result = NodeResolutionPass::run(std::move(map));

   // The MapNode is retained; its unresolved child was resolved into a concrete node.
   ASSERT_EQ(result->kind(), operators::NodeKind::MAP);
   auto* resolved_map = dynamic_cast<operators::MapNode*>(result.get());
   EXPECT_EQ(resolved_map->child->kind(), operators::NodeKind::INSERTIONS_NUCLEOTIDE);
}

TEST(NodeResolutionPassMap, propagatesChildErrorThroughMap) {
   auto unresolved = std::make_unique<operators::UnresolvedInsertionsNode<Nucleotide>>(
      makeNonScanChild(), std::vector<std::string>{}
   );
   auto map = std::make_unique<operators::MapNode>(std::move(unresolved), makeMapAssignments());
   EXPECT_THAT(
      [&]() { (void)NodeResolutionPass::run(std::move(map)); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("insertions() must be applied to a table scan")
      )
   );
}

}  // namespace
