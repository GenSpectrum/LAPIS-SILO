#ifndef SILO_NUC_MUTATIONS_H
#define SILO_NUC_MUTATIONS_H

#include <vector>

#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/actions/action.h"
#include "silo/storage/sequence_store.h"

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

  public:
   static constexpr size_t MUTATION_SYMBOL_COUNT = NucMutations::VALID_MUTATION_SYMBOLS.size();
   static constexpr double DEFAULT_MIN_PROPORTION = 0.02;

  private:
   static std::array<std::vector<uint32_t>, MUTATION_SYMBOL_COUNT> calculateMutationsPerPosition(
      const SequenceStore& seq_store,
      std::vector<OperatorResult>& bitmap_filter
   );

  public:
   explicit NucMutations(std::optional<std::string> nuc_sequence_name, double min_proportion);

   [[nodiscard]] QueryResult execute(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<NucMutations>& action);

}  // namespace silo::query_engine::actions

#endif  // SILO_NUC_MUTATIONS_H
