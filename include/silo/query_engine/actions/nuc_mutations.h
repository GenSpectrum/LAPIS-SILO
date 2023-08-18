#ifndef SILO_NUC_MUTATIONS_H
#define SILO_NUC_MUTATIONS_H

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
class SequenceStore;

namespace query_engine {
struct OperatorResult;
}  // namespace query_engine
}  // namespace silo

namespace silo::query_engine::actions {

class NucMutations : public Action {
   std::optional<std::string> nuc_sequence_name;
   double min_proportion;

   static constexpr std::array<NUCLEOTIDE_SYMBOL, 5> VALID_MUTATION_SYMBOLS{
      NUCLEOTIDE_SYMBOL::GAP,
      NUCLEOTIDE_SYMBOL::A,
      NUCLEOTIDE_SYMBOL::C,
      NUCLEOTIDE_SYMBOL::G,
      NUCLEOTIDE_SYMBOL::T,
   };

   const std::string POSITION_FIELD_NAME = "position";
   const std::string PROPORTION_FIELD_NAME = "proportion";
   const std::string COUNT_FIELD_NAME = "count";

   struct PrefilteredBitmaps {
      std::vector<std::pair<OperatorResult, const silo::SequenceStorePartition&>> bitmaps;
      std::vector<std::pair<OperatorResult, const silo::SequenceStorePartition&>> full_bitmaps;
   };

  public:
   static constexpr double DEFAULT_MIN_PROPORTION = 0.05;

  private:
   static PrefilteredBitmaps preFilterBitmaps(
      const silo::SequenceStore& seq_store,
      std::vector<OperatorResult>& bitmap_filter
   );

   static void addMutationsCountsForPosition(
      uint32_t position,
      PrefilteredBitmaps& bitmaps_to_evaluate,
      SymbolMap<NUCLEOTIDE_SYMBOL, std::vector<uint32_t>>& count_of_mutations_per_position
   );

   static SymbolMap<NUCLEOTIDE_SYMBOL, std::vector<uint32_t>> calculateMutationsPerPosition(
      const SequenceStore& seq_store,
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

#endif  // SILO_NUC_MUTATIONS_H
