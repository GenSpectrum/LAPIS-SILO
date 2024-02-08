#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/common/symbol_map.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/query_result.h"

namespace silo {
class Database;
template <typename SymbolType>
class SequenceStore;
template <typename SymbolType>
class SequenceStorePartition;
}  // namespace silo
namespace silo::query_engine {
struct OperatorResult;
}  // namespace silo::query_engine

namespace silo::query_engine::actions {

template <typename SymbolType>
class Mutations : public Action {
   std::vector<std::string> sequence_names;
   double min_proportion;

   const std::string MUTATION_FIELD_NAME = "mutation";
   const std::string SEQUENCE_FIELD_NAME = "sequenceName";
   const std::string PROPORTION_FIELD_NAME = "proportion";
   const std::string COUNT_FIELD_NAME = "count";

   struct PrefilteredBitmaps {
      std::vector<std::pair<const OperatorResult&, const silo::SequenceStorePartition<SymbolType>&>>
         bitmaps;
      std::vector<std::pair<const OperatorResult&, const silo::SequenceStorePartition<SymbolType>&>>
         full_bitmaps;
   };

   static std::unordered_map<std::string, Mutations<SymbolType>::PrefilteredBitmaps>
   preFilterBitmaps(const silo::Database& database, std::vector<OperatorResult>& bitmap_filter);

   static void addPositionToMutationCountsForMixedBitmaps(
      uint32_t position_idx,
      const PrefilteredBitmaps& bitmaps_to_evaluate,
      SymbolMap<SymbolType, std::vector<uint32_t>>& count_of_mutations_per_position
   );

   static void addPositionToMutationCountsForFullBitmaps(
      uint32_t position_idx,
      const PrefilteredBitmaps& bitmaps_to_evaluate,
      SymbolMap<SymbolType, std::vector<uint32_t>>& count_of_mutations_per_position
   );

   static SymbolMap<SymbolType, std::vector<uint32_t>> calculateMutationsPerPosition(
      const SequenceStore<SymbolType>& sequence_store,
      const PrefilteredBitmaps& bitmap_filter
   );

   void addMutationsToOutput(
      const std::string& sequence_name,
      const SequenceStore<SymbolType>& sequence_store,
      const PrefilteredBitmaps& bitmap_filter,
      std::vector<QueryResultEntry>& output
   ) const;

   [[nodiscard]] void validateOrderByFields(const Database& database) const override;

   [[nodiscard]] QueryResult execute(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const override;

  public:
   explicit Mutations(std::vector<std::string>&& aa_sequence_names, double min_proportion);
};

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Mutations<SymbolType>>& action);

}  // namespace silo::query_engine::actions
