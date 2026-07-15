#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include <type_traits>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/table.h"

namespace silo {
class Nucleotide;
}

namespace silo::query_engine::expressions {

template <typename SymbolType>
class MutationProfile : public Expression {
  public:
   struct Mutation {
      uint32_t position_idx;  // 0-indexed
      typename SymbolType::Symbol symbol;
   };

   struct QuerySequenceInput {
      std::string sequence;
   };

   struct SequenceIdInput {
      std::string id;
   };

   struct MutationsInput {
      std::vector<Mutation> mutations;
   };

   using ProfileInput = std::variant<QuerySequenceInput, SequenceIdInput, MutationsInput>;

  private:
   std::optional<std::string> sequence_name;
   uint32_t distance;
   ProfileInput input;

   [[nodiscard]] std::vector<typename SymbolType::Symbol> buildProfileFromQuerySequence(
      const storage::column::SequenceColumn<SymbolType>& sequence_column
   ) const;

   [[nodiscard]] std::vector<typename SymbolType::Symbol> buildProfileFromSequenceId(
      const storage::Table& table,
      const std::string& valid_sequence_name
   ) const;

   [[nodiscard]] std::vector<typename SymbolType::Symbol> buildProfileFromMutations(
      const storage::column::SequenceColumn<SymbolType>& sequence_column
   ) const;

  public:
   explicit MutationProfile(
      std::optional<std::string> sequence_name,
      uint32_t distance,
      ProfileInput input
   );

   [[nodiscard]] std::unique_ptr<Expression> clone() const override {
      return std::make_unique<MutationProfile>(sequence_name, distance, input);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = std::is_same_v<SymbolType, Nucleotide>
                                   ? Kind::MUTATION_PROFILE_NUCLEOTIDE
                                   : Kind::MUTATION_PROFILE_AMINO_ACID;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::vector<schema::ColumnIdentifier> freeIUs() const override {
      const auto col_type = std::is_same_v<SymbolType, Nucleotide>
                               ? schema::ColumnType::NUCLEOTIDE_SEQUENCE
                               : schema::ColumnType::AMINO_ACID_SEQUENCE;
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
