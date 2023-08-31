#pragma once

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
#include "silo/storage/sequence_store.h"

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

   static constexpr std::array<AminoAcid::Symbol, 20> VALID_MUTATION_SYMBOLS{
      AminoAcid::Symbol::A,  // Alanine
      AminoAcid::Symbol::C,  // Cysteine
      AminoAcid::Symbol::D,  // Aspartic Acid
      AminoAcid::Symbol::E,  // Glutamic Acid
      AminoAcid::Symbol::F,  // Phenylalanine
      AminoAcid::Symbol::G,  // Glycine
      AminoAcid::Symbol::H,  // Histidine
      AminoAcid::Symbol::I,  // Isoleucine
      AminoAcid::Symbol::K,  // Lysine
      AminoAcid::Symbol::L,  // Leucine
      AminoAcid::Symbol::M,  // Methionine
      AminoAcid::Symbol::N,  // Asparagine
      AminoAcid::Symbol::P,  // Proline
      AminoAcid::Symbol::Q,  // Glutamine
      AminoAcid::Symbol::R,  // Arginine
      AminoAcid::Symbol::S,  // Serine
      AminoAcid::Symbol::T,  // Threonine
      AminoAcid::Symbol::V,  // Valine
      AminoAcid::Symbol::W,  // Tryptophan
      AminoAcid::Symbol::Y,  // Tyrosine
   };

   const std::string MUTATION_FIELD_NAME = "mutation";
   const std::string SEQUENCE_FIELD_NAME = "sequenceName";
   const std::string PROPORTION_FIELD_NAME = "proportion";
   const std::string COUNT_FIELD_NAME = "count";

   struct PrefilteredBitmaps {
      std::vector<std::pair<const OperatorResult&, const silo::SequenceStorePartition<AminoAcid>&>>
         bitmaps;
      std::vector<std::pair<const OperatorResult&, const silo::SequenceStorePartition<AminoAcid>&>>
         full_bitmaps;
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
      SymbolMap<AminoAcid, std::vector<uint32_t>>& count_of_mutations_per_position
   );

   static SymbolMap<AminoAcid, std::vector<uint32_t>> calculateMutationsPerPosition(
      const SequenceStore<AminoAcid>& aa_store,
      const PrefilteredBitmaps& bitmap_filter
   );

   void addMutationsToOutput(
      const std::string& sequence_name,
      const SequenceStore<AminoAcid>& aa_store,
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
