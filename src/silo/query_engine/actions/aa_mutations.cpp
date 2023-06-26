#include "silo/query_engine/actions/aa_mutations.h"

#include <nlohmann/json.hpp>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_result.h"

namespace silo::query_engine::actions {

AAMutations::AAMutations() = default;

QueryResult AAMutations::execute(
   const Database& database,
   std::vector<OperatorResult> bitmap_filter
) const {
   return {ErrorResult{"Not implemented", "The AAMutations action has not been implemented"}};
}

void from_json(const nlohmann::json& /*json*/, std::unique_ptr<AAMutations>& action) {
   action = std::make_unique<AAMutations>();
}

}  // namespace silo::query_engine::actions
