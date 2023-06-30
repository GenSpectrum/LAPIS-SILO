#ifndef SILO_NUC_MUTATIONS_H
#define SILO_NUC_MUTATIONS_H

#include "silo/query_engine/actions/action.h"

#include <vector>

namespace silo::query_engine::actions {

class NucMutations : public Action {
   double min_proportion;

  public:
   explicit NucMutations(double min_proportion);

   [[nodiscard]] QueryResult execute(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<NucMutations>& action);

}  // namespace silo::query_engine::actions

#endif  // SILO_NUC_MUTATIONS_H
