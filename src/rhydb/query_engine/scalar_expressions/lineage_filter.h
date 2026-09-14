#pragma once

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "rhydb/common/lineage_tree.h"
#include "rhydb/query_engine/filter/operators/operator.h"
#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"
#include "rhydb/schema/database_schema.h"

namespace rhydb::query_engine::scalar_expressions {

/// `lineage(column, value, ...)`, before it knows the lineage hierarchy. The hierarchy lives in a
/// lineage relation table, which `lineage_definition` names, so this expression is resolved in
/// `rewrite()` - where the filtered table, and through it the definition, is available - into the
/// primitives that do the matching: an IsNull, an equality, or a StringInSet over the lineage and
/// its sublineages. `compile()` is therefore never reached.
class LineageFilter : public ScalarExpression {
   schema::ColumnIdentifier column;
   std::optional<std::string> lineage;
   std::optional<rhydb::common::RecombinantEdgeFollowingMode> sublineage_mode;
   std::string lineage_definition;

  public:
   explicit LineageFilter(
      schema::ColumnIdentifier column,
      std::optional<std::string> lineage,
      std::optional<rhydb::common::RecombinantEdgeFollowingMode> sublineage_mode,
      std::string lineage_definition
   );

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<LineageFilter>(column, lineage, sublineage_mode, lineage_definition);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::LINEAGE_FILTER;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::vector<schema::ColumnIdentifier> freeIUs() const override;

   [[nodiscard]] std::unique_ptr<ScalarExpression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace rhydb::query_engine::scalar_expressions
