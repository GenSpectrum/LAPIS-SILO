#pragma once

#include <memory>
#include <string>
#include <vector>

#include "rhydb/query_engine/filter/operators/operator.h"
#include "rhydb/query_engine/filter/operators/selection.h"
#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"
#include "rhydb/schema/database_schema.h"

namespace rhydb::query_engine::scalar_expressions {

/// Ordering comparison predicate: `left <op> right`. One side is a column
/// reference (FieldRef) and the other a literal value; compile() recognises this
/// "column <op> constant" shape and lowers it to an efficient filter, dispatching
/// on the literal's type.
///
/// Handles strict/inclusive ordering comparisons (<, >, <=, >=). Equality (=) and
/// inequality (<>) remain owned by Equals + Negation (issues #1435/#1437 scope
/// boundary); comparator is stored generically so a future fold is trivial, but
/// only the 4 ordering comparators are produced today.
class Comparison : public ScalarExpression {
   std::unique_ptr<ScalarExpression> left;
   std::unique_ptr<ScalarExpression> right;
   filter::operators::Comparator comparator;

  public:
   Comparison(
      std::unique_ptr<ScalarExpression> left,
      std::unique_ptr<ScalarExpression> right,
      filter::operators::Comparator comparator
   );

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<Comparison>(left->clone(), right->clone(), comparator);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::COMPARISON;
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
