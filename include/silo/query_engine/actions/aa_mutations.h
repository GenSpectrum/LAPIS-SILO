#ifndef SILO_AA_MUTATIONS_H
#define SILO_AA_MUTATIONS_H

#include "silo/common/aa_symbols.h"
#include "silo/query_engine/actions/action.h"
#include "silo/storage/aa_store.h"

namespace silo::query_engine::actions {

class AAMutations : public Action {
   std::string aa_sequence_name;
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

  public:
   static constexpr size_t MUTATION_SYMBOL_COUNT = AAMutations::VALID_MUTATION_SYMBOLS.size();
   static constexpr double DEFAULT_MIN_PROPORTION = 0.02;

  private:
   static std::array<std::vector<uint32_t>, AAMutations::MUTATION_SYMBOL_COUNT>
   calculateMutationsPerPosition(
      const AAStore& aa_store,
      std::vector<OperatorResult>& bitmap_filter
   );

  public:
   explicit AAMutations(std::string aa_sequence_name, double min_proportion);

   QueryResult execute(const Database& database, std::vector<OperatorResult> bitmap_filter)
      const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<AAMutations>& action);

}  // namespace silo::query_engine::actions

#endif  // SILO_AA_MUTATIONS_H
