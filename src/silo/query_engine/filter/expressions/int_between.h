#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::expressions {

class IntBetween : public Expression {
  private:
   std::string column_name;
   std::optional<uint32_t> from;
   std::optional<uint32_t> to;

  public:
   explicit IntBetween(
      std::string column_name,
      std::optional<uint32_t> from,
      std::optional<uint32_t> to
   );

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::filter::expressions
