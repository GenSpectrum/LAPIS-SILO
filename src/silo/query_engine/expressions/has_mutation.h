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
class HasMutation : public Expression {
  private:
   std::optional<std::string> sequence_name;
   uint32_t position_idx;

  public:
   explicit HasMutation(std::optional<std::string> sequence_name, uint32_t position_idx);

   [[nodiscard]] std::unique_ptr<Expression> clone() const override {
      return std::make_unique<HasMutation>(sequence_name, position_idx);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = std::is_same_v<SymbolType, Nucleotide>
                                   ? Kind::HAS_MUTATION_NUCLEOTIDE
                                   : Kind::HAS_MUTATION_AMINO_ACID;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::vector<schema::ColumnIdentifier> freeIUs() const override {
      const auto col_type = std::is_same_v<SymbolType, Nucleotide>
                               ? schema::ColumnType::NUCLEOTIDE_SEQUENCE
                               : schema::ColumnType::AMINO_ACID_SEQUENCE;
      // When sequence_name is nullopt the actual name is resolved from the schema at
      // rewrite() time; we return an empty name as a safe placeholder (it cannot match
      // any real column name in a MapNode assignment).
      return {{.name = sequence_name.value_or(""), .type = col_type}};
   }

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::expressions
