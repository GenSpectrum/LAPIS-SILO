#pragma once

#include <memory>
#include <string>
#include <vector>

#include "rhydb/query_engine/filter/operators/operator.h"
#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"
#include "rhydb/schema/database_schema.h"

namespace rhydb::query_engine::scalar_expressions {

/// Equality filter predicate: `left = right`.
///
/// In practice one side is a column reference (FieldRef) and the other a
/// literal value. compile() recognises this "column = constant" shape and
/// lowers it to an efficient bitmap filter, dispatching on the literal's type;
/// any other shape cannot currently be compiled.
class Equals : public ScalarExpression {
   std::unique_ptr<ScalarExpression> left;
   std::unique_ptr<ScalarExpression> right;

  public:
   Equals(std::unique_ptr<ScalarExpression> left, std::unique_ptr<ScalarExpression> right);

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<Equals>(left->clone(), right->clone());
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::EQUALS;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::vector<schema::ColumnIdentifier> freeIUs() const override;

   [[nodiscard]] std::unique_ptr<ScalarExpression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace rhydb::query_engine::scalar_expressions
