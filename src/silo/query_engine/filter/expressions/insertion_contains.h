#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::expressions {

template <typename SymbolType>
class InsertionContains : public Expression {
  private:
   std::optional<std::string> sequence_name;
   uint32_t position_idx;
   std::string value;

  public:
   explicit InsertionContains(
      std::optional<std::string> sequence_name,
      uint32_t position_idx,
      std::string value
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
