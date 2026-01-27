#pragma once

#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

class Or : public Expression {
   ExpressionVector children;

  public:
   explicit Or(ExpressionVector&& children);

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      const storage::TablePartition& table_partition,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] static std::vector<const Expression*> collectChildren(
      const ExpressionVector& children
   );

   [[nodiscard]] static ExpressionVector algebraicSimplification(ExpressionVector children);

   template <typename SymbolType>
   [[nodiscard]] static ExpressionVector rewriteSymbolInSetExpressions(ExpressionVector children);

   [[nodiscard]] static ExpressionVector mergeStringInSetExpressions(ExpressionVector children);

   [[nodiscard]] std::unique_ptr<operators::Operator> compile(
      const storage::Table& table,
      const storage::TablePartition& table_partition
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Or>& filter);

}  // namespace silo::query_engine::filter::expressions
