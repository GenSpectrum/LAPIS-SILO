#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <arrow/result.h>

#include "silo/common/lineage_tree.h"
#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// Computes, for every lineage defined for a lineage-indexed column, the number of filtered rows
/// whose lineage is that lineage OR any of its sublineages (per the given recombinant-following
/// mode). Emits one row per defined lineage, including lineages with count 0.
///
/// This is NOT a partition of the filtered rows: a row in lineage `A.1` is counted under both
/// `A.1` and its ancestor `A`, so the emitted counts do not sum to the filtered total.
///
/// Implementation: intersect the query filter bitmap with the precomputed, DAG-correct
/// `index_including_sublineages` bitmaps in the column's LineageIndex (the same bitmaps the
/// `lineage(..., includeSublineages := true)` filter uses). Do NOT replace this with a
/// tree-rollup of exclusive counts: the lineage structure is a DAG (recombinants have multiple
/// parents), so summing child counts would double-count.
class LineageAggregateNode final : public QueryNode {
  public:
   std::shared_ptr<storage::Table> table;
   std::unique_ptr<expressions::Expression> filter;
   std::string column_name;
   common::RecombinantEdgeFollowingMode mode;
   std::string count_output_name;

   LineageAggregateNode(
      std::shared_ptr<storage::Table> table,
      std::unique_ptr<expressions::Expression> filter,
      std::string column_name,
      common::RecombinantEdgeFollowingMode mode,
      std::string count_output_name
   );

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<arrow::acero::ExecNode*> addToExecPlan(
      arrow::acero::ExecPlan& plan,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;

   [[nodiscard]] NodeKind kind() const override { return NodeKind::LINEAGE_AGGREGATE; }

   [[nodiscard]] nlohmann::json toJson() const override;
};

}  // namespace silo::query_engine::operators
