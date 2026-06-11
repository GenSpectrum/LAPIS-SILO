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

class Or;

template <typename SymbolType>
class SymbolInSet : public Expression {
   friend class Or;  // For optimization under Or

   std::optional<std::string> sequence_name;
   uint32_t position_idx;
   std::vector<typename SymbolType::Symbol> symbols;

  public:
   explicit SymbolInSet(
      std::optional<std::string> sequence_name,
      uint32_t position_idx,
      std::vector<typename SymbolType::Symbol> symbols
   );

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = std::is_same_v<SymbolType, Nucleotide>
                                   ? Kind::SYMBOL_IN_SET_NUCLEOTIDE
                                   : Kind::SYMBOL_IN_SET_AMINO_ACID;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;

  private:
   static std::string getFilterName() {
      return fmt::format("SymbolInSet<{}>", SymbolType::SYMBOL_NAME);
   }
};

}  // namespace silo::query_engine::expressions
