#pragma once

#include <memory>
#include <string>
#include <tuple>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::expressions {

class NOf : public Expression {
  private:
   ExpressionVector children;
   int number_of_matchers;
   bool match_exactly;

   [[nodiscard]] ExpressionVector rewriteChildren(
      const storage::Table& table,
      Expression::AmbiguityMode mode
   ) const;

   [[nodiscard]] std::unique_ptr<Expression> rewriteToNonExact(
      const storage::Table& table,
      Expression::AmbiguityMode mode
   ) const;

   [[nodiscard]] std::
      tuple<filter::operators::OperatorVector, filter::operators::OperatorVector, int>
      mapChildExpressions(const storage::Table& table) const;

  public:
   explicit NOf(ExpressionVector&& children, int number_of_matchers, bool match_exactly);

   [[nodiscard]] std::unique_ptr<Expression> clone() const override {
      ExpressionVector cloned;
      cloned.reserve(children.size());
      for (const auto& child : children) {
         cloned.push_back(child->clone());
      }
      return std::make_unique<NOf>(std::move(cloned), number_of_matchers, match_exactly);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::N_OF;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::vector<schema::ColumnIdentifier> freeIUs() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::expressions
