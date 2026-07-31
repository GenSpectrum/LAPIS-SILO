#pragma once

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::scalar_expressions {

class Negation : public ScalarExpression {
   friend class ScalarExpression;

  private:
   std::unique_ptr<ScalarExpression> child;

  public:
   explicit Negation(std::unique_ptr<ScalarExpression> child);

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<Negation>(child->clone());
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::NEGATION;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::vector<schema::ColumnIdentifier> freeIUs() const override;

   [[nodiscard]] std::unique_ptr<ScalarExpression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::scalar_expressions
