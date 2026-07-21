#pragma once

#include <memory>
#include <string>
#include <unordered_set>

#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"

namespace silo::query_engine::scalar_expressions {

class Or;

class StringInSet : public ScalarExpression {
   friend class Or;

  private:
   std::string column_name;
   std::unordered_set<std::string> values;

  public:
   explicit StringInSet(std::string column_name, std::unordered_set<std::string> value);

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<StringInSet>(column_name, values);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::STRING_IN_SET;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<ScalarExpression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::scalar_expressions
