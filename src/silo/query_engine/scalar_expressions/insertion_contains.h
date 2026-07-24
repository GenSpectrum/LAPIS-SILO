#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include <type_traits>

#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"

namespace silo {
class Nucleotide;
}

namespace silo::query_engine::scalar_expressions {

template <typename SymbolType>
class InsertionContains : public ScalarExpression {
  private:
   std::string sequence_name;
   uint32_t position_idx;
   std::string value;

  public:
   explicit InsertionContains(std::string sequence_name, uint32_t position_idx, std::string value);

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<InsertionContains>(sequence_name, position_idx, value);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = std::is_same_v<SymbolType, Nucleotide>
                                   ? Kind::INSERTION_CONTAINS_NUCLEOTIDE
                                   : Kind::INSERTION_CONTAINS_AMINO_ACID;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<ScalarExpression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::scalar_expressions
