#ifndef SILO_AA_MUTATIONS_H
#define SILO_AA_MUTATIONS_H

#include "silo/query_engine/actions/action.h"

namespace silo::query_engine::actions {

class AAMutations : public Action {
  public:
   explicit AAMutations();

   QueryResult execute(const Database& database, std::vector<OperatorResult> bitmap_filter)
      const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<AAMutations>& action);

}  // namespace silo::query_engine::actions

#endif  // SILO_AA_MUTATIONS_H
