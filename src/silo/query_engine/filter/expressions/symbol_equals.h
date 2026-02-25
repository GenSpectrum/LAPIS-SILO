#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::expressions {

template <typename SymbolType>
class SymbolOrDot {
   std::optional<typename SymbolType::Symbol> value;

   SymbolOrDot() = default;

  public:
   static SymbolOrDot<SymbolType> dot();

   explicit SymbolOrDot(typename SymbolType::Symbol symbol);

   [[nodiscard]] typename SymbolType::Symbol getSymbolOrReplaceDotWith(
      typename SymbolType::Symbol replace_dot_with
   ) const;

   [[nodiscard]] char asChar() const;
};

template <typename SymbolType>
class SymbolEquals : public Expression {
   std::optional<std::string> sequence_name;
   uint32_t position_idx;
   SymbolOrDot<SymbolType> value;

  public:
   explicit SymbolEquals(
      std::optional<std::string> sequence_name,
      uint32_t position_idx,
      SymbolOrDot<SymbolType> value
   );

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<operators::Operator> compile(const storage::Table& table
   ) const override;

  private:
   static std::string getFilterName() {
      return fmt::format("SymbolEquals<{}>", SymbolType::SYMBOL_NAME);
   }
};

}  // namespace silo::query_engine::filter::expressions
