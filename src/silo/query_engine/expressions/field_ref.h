#pragma once

#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::expressions {

/// References an existing column by name. As a scalar expression it evaluates to
/// that column's value per row, e.g. `y := age`; its type() is the referenced
/// column's type. It is not a filter predicate, so it cannot be compile()d into a
/// filter operator.
class FieldRef : public Expression {
  public:
   schema::ColumnIdentifier column;

   explicit FieldRef(schema::ColumnIdentifier column);

   [[nodiscard]] schema::ColumnType type() const override { return column.type; }

   [[nodiscard]] std::string toString() const override;
   [[nodiscard]] bool operator==(const Expression& other) const override;

   [[nodiscard]] std::vector<schema::ColumnIdentifier> freeIUs() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::expressions
