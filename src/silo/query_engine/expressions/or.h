#pragma once

#include <memory>
#include <string>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::expressions {

class Or : public Expression {
   ExpressionVector children;

  public:
   explicit Or(ExpressionVector&& children);

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::OR;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] static std::vector<const Expression*> collectChildren(
      const ExpressionVector& children
   );

   [[nodiscard]] static ExpressionVector algebraicSimplification(
      ExpressionVector unprocessed_child_expressions
   );

   template <typename SymbolType>
   [[nodiscard]] static ExpressionVector rewriteSymbolInSetExpressions(ExpressionVector children);

   [[nodiscard]] static ExpressionVector mergeStringInSetExpressions(ExpressionVector children);

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::expressions
