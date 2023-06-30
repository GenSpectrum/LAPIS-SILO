#include "silo/query_engine/actions/fasta_aligned.h"

#include <nlohmann/json.hpp>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_result.h"

namespace silo::query_engine::actions {

FastaAligned::FastaAligned() = default;

QueryResult FastaAligned::execute(
   const Database& database,
   std::vector<OperatorResult> bitmap_filter
) const {
   return {ErrorResult{"Not implemented", "The FastAligned action has not been implemented"}};
}

void from_json(const nlohmann::json& /*json*/, std::unique_ptr<FastaAligned>& action) {
   action = std::make_unique<FastaAligned>();
}

}  // namespace silo::query_engine::actions
