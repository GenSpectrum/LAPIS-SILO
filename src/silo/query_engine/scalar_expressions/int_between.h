#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::scalar_expressions {

class IntBetween : public ScalarExpression {
  private:
   schema::ColumnIdentifier column;
   std::optional<int32_t> from;
   std::optional<int32_t> to;

  public:
   explicit IntBetween(
      schema::ColumnIdentifier column,
      std::optional<int32_t> from,
      std::optional<int32_t> to
   );

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<IntBetween>(column, from, to);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::INT_BETWEEN;
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
