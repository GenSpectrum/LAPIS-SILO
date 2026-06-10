#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/table.h"

namespace silo::query_engine::expressions {

template <typename SymbolType>
class MutationProfile : public Expression {
  public:
   struct Mutation {
      uint32_t position_idx;  // 0-indexed
      typename SymbolType::Symbol symbol;

      bool operator==(const Mutation& other) const = default;
   };

   struct QuerySequenceInput {
      std::string sequence;

      bool operator==(const QuerySequenceInput& other) const = default;
   };

   struct SequenceIdInput {
      std::string id;

      bool operator==(const SequenceIdInput& other) const = default;
   };

   struct MutationsInput {
      std::vector<Mutation> mutations;

      bool operator==(const MutationsInput& other) const = default;
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

   [[nodiscard]] std::string toString() const override;
   [[nodiscard]] bool operator==(const Expression& other) const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::expressions
