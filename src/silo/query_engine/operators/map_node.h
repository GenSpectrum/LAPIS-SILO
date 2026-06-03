#pragma once

#include <map>
#include <memory>
#include <vector>

#include <arrow/result.h>
#include <arrow/scalar.h>

#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// Extends its child's output with additional columns. Each added column is
/// assigned a literal value (e.g. `x := 3`). All of the child's columns are
/// passed through unchanged; an assignment whose output name matches an
/// existing column replaces that column in place.
class MapNode final : public QueryNode {
  public:
   /// A literal value assigned to a new column, e.g. `x := 3`.
   struct Int64Literal {
      int64_t value;
   };
   struct FloatLiteral {
      double value;
   };
   struct StringLiteral {
      std::string value;
   };
   struct BoolLiteral {
      bool value;
   };

   struct Assignment {
      schema::ColumnIdentifier output_column;
      std::variant<Int64Literal, FloatLiteral, StringLiteral, BoolLiteral> expression;
   };

   QueryNodePtr child;
   std::vector<Assignment> assignments;

   MapNode(QueryNodePtr child, std::vector<Assignment> assignments);

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;

   [[nodiscard]] NodeKind kind() const override { return NodeKind::MAP; }
};

}  // namespace silo::query_engine::operators
