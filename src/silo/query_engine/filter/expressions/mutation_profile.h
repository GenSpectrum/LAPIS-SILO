#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/table.h"

namespace silo::query_engine::filter::expressions {

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

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<operators::Operator> compile(const storage::Table& table
   ) const override;
};

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<MutationProfile<SymbolType>>& filter);

}  // namespace silo::query_engine::filter::expressions
