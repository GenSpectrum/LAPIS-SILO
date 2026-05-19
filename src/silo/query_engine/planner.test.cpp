#include "silo/query_engine/planner.h"

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <arrow/result.h>
#include <arrow/status.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/config/runtime_config.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/operators/scan_node.h"
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
using silo::query_engine::Planner;
namespace operators = silo::query_engine::operators;

namespace {

using Tables = std::map<silo::schema::TableName, std::shared_ptr<silo::storage::Table>>;

Tables makeTablesWithDefault() {
   using silo::schema::ColumnIdentifier;
   using silo::schema::ColumnType;
   using silo::storage::column::ColumnMetadata;
   using silo::storage::column::StringColumnMetadata;

   ColumnIdentifier primary_key{.name = "id", .type = ColumnType::STRING};
   std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> col_meta{
      {primary_key, std::make_shared<StringColumnMetadata>(primary_key.name)}
   };
   auto schema = std::make_shared<silo::schema::TableSchema>(std::move(col_meta), primary_key);
   Tables tables;
   tables[silo::schema::TableName("default")] = std::make_shared<silo::storage::Table>(schema);
   return tables;
}

operators::QueryNodePtr makeNonScanChild() {
   return std::make_unique<operators::UnresolvedMutationsNode<Nucleotide>>(
      std::make_unique<operators::ScanNode>(
         silo::schema::TableName{"default"}, std::vector<silo::schema::ColumnIdentifier>{}
      ),
      std::vector<std::string>{},
      0.0,
      std::vector<std::string>{}
   );
}

class ErrorQueryNode final : public operators::QueryNode {
  public:
   [[nodiscard]] std::vector<silo::schema::ColumnIdentifier> getOutputSchema() const override {
      return {};
   }

   [[nodiscard]] arrow::Result<operators::PartialArrowPlan> toQueryPlan(
      const std::map<silo::schema::TableName, std::shared_ptr<silo::storage::Table>>& /*tables*/,
      const silo::config::QueryOptions& /*query_options*/
   ) const override {
      return arrow::Status::ExecutionError("induced test error");
   }

   [[nodiscard]] operators::NodeKind kind() const override {
      return operators::NodeKind::TABLE_SCAN;
   }
};

// --- resolveTable ---

TEST(PlannerResolveTable, tableNotFoundThrows) {
   auto node = std::make_unique<operators::ScanNode>(
      silo::schema::TableName{"missingtable"}, std::vector<silo::schema::ColumnIdentifier>{}
   );
   EXPECT_THAT(
      [&]() { (void)Planner::pushdown(std::move(node), {}); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("table 'missingtable' not found in database")
      )
   );
}

// --- mutations() pushdown ---

TEST(PlannerMutations, onNonScanNodeThrows) {
   auto non_scan = makeNonScanChild();
   auto node = std::make_unique<operators::UnresolvedMutationsNode<Nucleotide>>(
      std::move(non_scan), std::vector<std::string>{}, 0.0, std::vector<std::string>{}
   );
   EXPECT_THAT(
      [&]() { (void)Planner::pushdown(std::move(node), {}); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("mutations() must be applied to a table scan")
      )
   );
}

TEST(PlannerMutations, invalidAASequenceNameThrows) {
   auto tables = makeTablesWithDefault();
   auto scan = std::make_unique<operators::ScanNode>(
      silo::schema::TableName{"default"}, std::vector<silo::schema::ColumnIdentifier>{}
   );
   std::vector<std::string> seq_names{"noseq"};
   auto node = std::make_unique<operators::UnresolvedMutationsNode<AminoAcid>>(
      std::move(scan), std::move(seq_names), 0.0, std::vector<std::string>{}
   );
   EXPECT_THAT(
      [&]() { (void)Planner::pushdown(std::move(node), tables); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("The database does not contain the AminoAcid sequence 'noseq'")
      )
   );
}

TEST(PlannerMutations, invalidFieldThrows) {
   auto tables = makeTablesWithDefault();
   auto scan = std::make_unique<operators::ScanNode>(
      silo::schema::TableName{"default"}, std::vector<silo::schema::ColumnIdentifier>{}
   );
   std::vector<std::string> bad_fields{"badfield"};
   auto node = std::make_unique<operators::UnresolvedMutationsNode<Nucleotide>>(
      std::move(scan), std::vector<std::string>{}, 0.0, std::move(bad_fields)
   );
   EXPECT_THAT(
      [&]() { (void)Planner::pushdown(std::move(node), tables); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("The attribute 'fields' contains an invalid field 'badfield'")
      )
   );
}

// --- insertions() pushdown ---

TEST(PlannerInsertions, onNonScanNodeThrows) {
   auto non_scan = makeNonScanChild();
   auto node = std::make_unique<operators::UnresolvedInsertionsNode<Nucleotide>>(
      std::move(non_scan), std::vector<std::string>{}
   );
   EXPECT_THAT(
      [&]() { (void)Planner::pushdown(std::move(node), {}); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("insertions() must be applied to a table scan")
      )
   );
}

TEST(PlannerInsertions, invalidAASequenceNameThrows) {
   auto tables = makeTablesWithDefault();
   auto scan = std::make_unique<operators::ScanNode>(
      silo::schema::TableName{"default"}, std::vector<silo::schema::ColumnIdentifier>{}
   );
   std::vector<std::string> seq_names{"noseq"};
   auto node = std::make_unique<operators::UnresolvedInsertionsNode<AminoAcid>>(
      std::move(scan), std::move(seq_names)
   );
   EXPECT_THAT(
      [&]() { (void)Planner::pushdown(std::move(node), tables); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("The database does not contain the AminoAcid sequence 'noseq'")
      )
   );
}

// --- phyloSubtree() pushdown ---

TEST(PlannerPhyloSubtree, onNonScanNodeThrows) {
   auto non_scan = makeNonScanChild();
   auto node = std::make_unique<operators::UnresolvedPhyloSubtreeNode>(
      std::move(non_scan), std::string{"col"}, false, false
   );
   EXPECT_THAT(
      [&]() { (void)Planner::pushdown(std::move(node), {}); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("phyloSubtree() must be applied to a table scan")
      )
   );
}

// --- mostRecentCommonAncestor() pushdown ---

TEST(PlannerMostRecentCommonAncestor, onNonScanNodeThrows) {
   auto non_scan = makeNonScanChild();
   auto node = std::make_unique<operators::UnresolvedMostRecentCommonAncestorNode>(
      std::move(non_scan), std::string{"col"}, false
   );
   EXPECT_THAT(
      [&]() { (void)Planner::pushdown(std::move(node), {}); },
      ThrowsMessage<IllegalQueryException>(
         ::testing::HasSubstr("mostRecentCommonAncestor() must be applied to a table scan")
      )
   );
}

// --- planQuery Arrow error path ---

TEST(PlannerPlanQuery, arrowErrorThrows) {
   silo::config::QueryOptions options{.materialization_cutoff = 1024};
   auto node = std::make_unique<ErrorQueryNode>();
   EXPECT_THAT(
      [&]() { (void)Planner::planQuery(std::move(node), {}, options, "test"); },
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr("Error when planning query execution"))
   );
}

}  // namespace
