#ifndef SILO_AA_MUTATIONS_H
#define SILO_AA_MUTATIONS_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/symbol_map.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/query_result.h"

namespace silo {
class AAStore;
class AAStorePartition;
}  // namespace silo
namespace silo {
class Database;
}  // namespace silo
namespace silo::query_engine {
struct OperatorResult;
}  // namespace silo::query_engine

namespace silo::query_engine::actions {

class AAMutations : public Action {
   std::vector<std::string> aa_sequence_names;
   double min_proportion;

   static constexpr std::array<AA_SYMBOL, 20> VALID_MUTATION_SYMBOLS{
      AA_SYMBOL::A,  // Alanine
      AA_SYMBOL::C,  // Cysteine
      AA_SYMBOL::D,  // Aspartic Acid
      AA_SYMBOL::E,  // Glutamic Acid
      AA_SYMBOL::F,  // Phenylalanine
      AA_SYMBOL::G,  // Glycine
      AA_SYMBOL::H,  // Histidine
      AA_SYMBOL::I,  // Isoleucine
      AA_SYMBOL::K,  // Lysine
      AA_SYMBOL::L,  // Leucine
      AA_SYMBOL::M,  // Methionine
      AA_SYMBOL::N,  // Asparagine
      AA_SYMBOL::P,  // Proline
      AA_SYMBOL::Q,  // Glutamine
      AA_SYMBOL::R,  // Arginine
      AA_SYMBOL::S,  // Serine
      AA_SYMBOL::T,  // Threonine
      AA_SYMBOL::V,  // Valine
      AA_SYMBOL::W,  // Tryptophan
      AA_SYMBOL::Y,  // Tyrosine
   };

   const std::string POSITION_FIELD_NAME = "position";
   const std::string SEQUENCE_FIELD_NAME = "sequenceName";
   const std::string PROPORTION_FIELD_NAME = "proportion";
   const std::string COUNT_FIELD_NAME = "count";

   struct PrefilteredBitmaps {
      std::vector<std::pair<const OperatorResult&, const silo::AAStorePartition&>> bitmaps;
      std::vector<std::pair<const OperatorResult&, const silo::AAStorePartition&>> full_bitmaps;
   };

  public:
   static constexpr double DEFAULT_MIN_PROPORTION = 0.05;

  private:
   static std::unordered_map<std::string, AAMutations::PrefilteredBitmaps> preFilterBitmaps(
      const silo::Database& database,
      std::vector<OperatorResult>& bitmap_filter
   );

   static void addMutationsCountsForPosition(
      uint32_t position,
      const PrefilteredBitmaps& bitmaps_to_evaluate,
      SymbolMap<AA_SYMBOL, std::vector<uint32_t>>& count_of_mutations_per_position
   );

   static SymbolMap<AA_SYMBOL, std::vector<uint32_t>> calculateMutationsPerPosition(
      const AAStore& aa_store,
      const PrefilteredBitmaps& bitmap_filter
   );

   void addMutationsToOutput(
      const std::string& sequence_name,
      const AAStore& aa_store,
      const PrefilteredBitmaps& bitmap_filter,
      std::vector<QueryResultEntry>& output
   ) const;

   [[nodiscard]] void validateOrderByFields(const Database& database) const override;

   [[nodiscard]] QueryResult execute(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const override;

  public:
   explicit AAMutations(std::vector<std::string>&& aa_sequence_names, double min_proportion);
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<AAMutations>& action);

}  // namespace silo::query_engine::actions

#endif  // SILO_AA_MUTATIONS_H
