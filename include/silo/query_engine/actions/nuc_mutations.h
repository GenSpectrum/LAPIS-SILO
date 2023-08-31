#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/sequence_store.h"

namespace silo {
class Database;

namespace query_engine {
struct OperatorResult;
}  // namespace query_engine
}  // namespace silo

namespace silo::query_engine::actions {

class NucMutations : public Action {
   std::optional<std::string> nuc_sequence_name;
   double min_proportion;

   static constexpr std::array<Nucleotide::Symbol, 5> VALID_MUTATION_SYMBOLS{
      Nucleotide::Symbol::GAP,
      Nucleotide::Symbol::A,
      Nucleotide::Symbol::C,
      Nucleotide::Symbol::G,
      Nucleotide::Symbol::T,
   };

   const std::string MUTATION_FIELD_NAME = "mutation";
   const std::string PROPORTION_FIELD_NAME = "proportion";
   const std::string COUNT_FIELD_NAME = "count";

   struct PrefilteredBitmaps {
      std::vector<std::pair<OperatorResult, const silo::SequenceStorePartition<Nucleotide>&>>
         bitmaps;
      std::vector<std::pair<OperatorResult, const silo::SequenceStorePartition<Nucleotide>&>>
         full_bitmaps;
   };

  public:
   static constexpr double DEFAULT_MIN_PROPORTION = 0.05;

  private:
   static PrefilteredBitmaps preFilterBitmaps(
      const silo::SequenceStore<Nucleotide>& seq_store,
      std::vector<OperatorResult>& bitmap_filter
   );

   static void addMutationsCountsForPosition(
      uint32_t position,
      PrefilteredBitmaps& bitmaps_to_evaluate,
      SymbolMap<Nucleotide, std::vector<uint32_t>>& count_of_mutations_per_position
   );

   static SymbolMap<Nucleotide, std::vector<uint32_t>> calculateMutationsPerPosition(
      const SequenceStore<Nucleotide>& seq_store,
      std::vector<OperatorResult>& bitmap_filter
   );

   [[nodiscard]] void validateOrderByFields(const Database& database) const override;

   [[nodiscard]] QueryResult execute(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const override;

  public:
   explicit NucMutations(std::optional<std::string> nuc_sequence_name, double min_proportion);
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<NucMutations>& action);

}  // namespace silo::query_engine::actions
