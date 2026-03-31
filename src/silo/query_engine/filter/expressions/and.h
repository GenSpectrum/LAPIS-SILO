#pragma once

#include <memory>
#include <string>
#include <tuple>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"

namespace silo::query_engine::filter::expressions {

class And : public Expression {
  private:
   ExpressionVector children;

   [[nodiscard]] std::
      tuple<operators::OperatorVector, operators::OperatorVector, operators::PredicateVector>
      compileChildren(const storage::Table& table) const;

  public:
   explicit And(ExpressionVector&& children);

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<operators::Operator> compile(const storage::Table& table
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<And>& filter);

}  // namespace silo::query_engine::filter::expressions
