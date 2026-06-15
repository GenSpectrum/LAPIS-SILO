#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include <type_traits>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo {
class Nucleotide;
}

namespace silo::query_engine::expressions {

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

   [[nodiscard]] std::unique_ptr<Expression> clone() const override {
      return std::make_unique<SymbolEquals>(sequence_name, position_idx, value);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = std::is_same_v<SymbolType, Nucleotide>
                                   ? Kind::SYMBOL_EQUALS_NUCLEOTIDE
                                   : Kind::SYMBOL_EQUALS_AMINO_ACID;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;

  private:
   static std::string getFilterName() {
      return fmt::format("SymbolEquals<{}>", SymbolType::SYMBOL_NAME);
   }
};

}  // namespace silo::query_engine::expressions
