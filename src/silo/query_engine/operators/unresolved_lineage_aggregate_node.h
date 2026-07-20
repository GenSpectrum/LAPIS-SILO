#pragma once

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "silo/common/lineage_tree.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::operators {

/// Placeholder for a sublineage-inclusive aggregate-count groupBy, resolved during pushdown.
///
/// Produced by handleGroupBy when the group-by spec is exactly one
/// `lineage(<column>, includeSublineages := true)` column plus a single `count()` aggregate.
/// Like UnresolvedMostRecentCommonAncestorNode it only carries the child tree; the flattened
/// (table, filter) that LineageAggregateNode needs is only available after the filter-pushdown
/// pass, so NodeResolutionPass turns this into a concrete LineageAggregateNode.
class UnresolvedLineageAggregateNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::string column_name;
   common::RecombinantEdgeFollowingMode mode;
   std::string count_output_name;

   UnresolvedLineageAggregateNode(
      QueryNodePtr child,
      std::string column_name,
      common::RecombinantEdgeFollowingMode mode,
      std::string count_output_name
   )
       : child(std::move(child)),
         column_name(std::move(column_name)),
         mode(mode),
         count_output_name(std::move(count_output_name)) {}

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override {
      return {
         {column_name, schema::ColumnType::STRING},
         {count_output_name, schema::ColumnType::INT64}
      };
   }

   [[nodiscard]] arrow::Result<arrow::acero::ExecNode*> addToExecPlan(
      arrow::acero::ExecPlan& /*plan*/,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
      const config::QueryOptions& /*query_options*/
   ) const override {
      throw std::runtime_error("UnresolvedLineageAggregateNode must be eliminated during pushdown");
   }

   [[nodiscard]] NodeKind kind() const override {
      return NodeKind::UNRESOLVED_LINEAGE_AGGREGATE;
   }

   [[nodiscard]] nlohmann::json toJson() const override {
      return {
         {"type", nodeKindToString(kind())},
         {"columnName", column_name},
         {"recombinantFollowingMode", static_cast<int>(mode)},
         {"countOutputName", count_output_name},
         {"child", child->toJson()},
      };
   }
};

}  // namespace silo::query_engine::operators
