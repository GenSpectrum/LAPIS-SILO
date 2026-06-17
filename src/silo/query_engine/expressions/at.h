#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::expressions {

/// A scalar expression that evaluates to the single character of a string-valued
/// column at a given 1-indexed position, e.g. `seq.at(3)`. Its value is a string
/// (the character), so it is not a filter predicate; compile() is unimplemented and
/// it is only meaningful as a scalar expression (e.g. in a map() assignment).
class At : public Expression {
  public:
   schema::ColumnIdentifier input_column;
   /// 1-indexed position of the character to extract.
   uint32_t position;

   At(schema::ColumnIdentifier input_column, uint32_t position);

   [[nodiscard]] schema::ColumnType type() const override { return schema::ColumnType::STRING; }

   static constexpr Kind KIND = Kind::AT;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::vector<schema::ColumnIdentifier> freeIUs() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<Expression> clone() const override {
      return std::make_unique<At>(input_column, position);
   }

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::expressions
