#ifndef SILO_FASTA_ALIGNED_H
#define SILO_FASTA_ALIGNED_H

#include "silo/query_engine/actions/action.h"

namespace silo::query_engine::actions {

class FastaAligned : public Action {
  public:
   explicit FastaAligned();

   QueryResult execute(const Database& database, std::vector<OperatorResult> bitmap_filter)
      const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FastaAligned>& action);

}  // namespace silo::query_engine::actions

#endif  // SILO_FASTA_ALIGNED_H
