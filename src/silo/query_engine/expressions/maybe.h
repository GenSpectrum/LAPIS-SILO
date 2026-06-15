#pragma once

#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::expressions {

class Maybe : public Expression {
   std::unique_ptr<Expression> child;

  public:
   explicit Maybe(std::unique_ptr<Expression> child);

   [[nodiscard]] std::unique_ptr<Expression> clone() const override {
      return std::make_unique<Maybe>(child->clone());
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::MAYBE;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::expressions
