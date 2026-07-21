#pragma once

#include <memory>
#include <string>
#include <tuple>

#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"

namespace silo::query_engine::scalar_expressions {

class And : public ScalarExpression {
  private:
   ScalarExpressionVector children;

   [[nodiscard]] std::tuple<
      filter::operators::OperatorVector,
      filter::operators::OperatorVector,
      filter::operators::PredicateVector>
   compileChildren(const storage::Table& table) const;

  public:
   explicit And(ScalarExpressionVector&& children);

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      ScalarExpressionVector cloned;
      cloned.reserve(children.size());
      for (const auto& child : children) {
         cloned.push_back(child->clone());
      }
      return std::make_unique<And>(std::move(cloned));
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::AND;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<ScalarExpression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::scalar_expressions
