#pragma once

#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::expressions {

class Negation : public Expression {
   friend class Expression;

  private:
   std::unique_ptr<Expression> child;

  public:
   explicit Negation(std::unique_ptr<Expression> child);

   [[nodiscard]] std::unique_ptr<Expression> clone() const override {
      return std::make_unique<Negation>(child->clone());
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::NEGATION;
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
