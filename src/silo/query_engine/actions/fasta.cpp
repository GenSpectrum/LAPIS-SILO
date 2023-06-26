#include "silo/query_engine/actions/fasta.h"

#include <nlohmann/json.hpp>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_result.h"

namespace silo::query_engine::actions {

Fasta::Fasta() = default;

QueryResult Fasta::execute(const Database& database, std::vector<OperatorResult> bitmap_filter)
   const {
   return {ErrorResult{"Not implemented", "The AAMutations action has not been implemented"}};
}

void from_json(const nlohmann::json& /*json*/, std::unique_ptr<Fasta>& action) {
   action = std::make_unique<Fasta>();
}

}  // namespace silo::query_engine::actions
