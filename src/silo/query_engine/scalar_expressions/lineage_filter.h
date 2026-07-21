#pragma once

#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"

namespace silo::query_engine::scalar_expressions {

class LineageFilter : public ScalarExpression {
   std::string column_name;
   std::optional<std::string> lineage;
   std::optional<silo::common::RecombinantEdgeFollowingMode> sublineage_mode;

  public:
   explicit LineageFilter(
      std::string column_name,
      std::optional<std::string> lineage,
      std::optional<silo::common::RecombinantEdgeFollowingMode> sublineage_mode
   );

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<LineageFilter>(column_name, lineage, sublineage_mode);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::LINEAGE_FILTER;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<ScalarExpression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;

  private:
   [[nodiscard]] std::optional<const roaring::Roaring*> getBitmapForValue(
      const silo::storage::column::IndexedStringColumn& lineage_column
   ) const;
};

}  // namespace silo::query_engine::scalar_expressions
