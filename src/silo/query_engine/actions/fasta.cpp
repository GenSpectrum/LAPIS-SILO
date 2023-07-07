#include "silo/query_engine/actions/fasta.h"

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_result.h"

namespace silo {
struct Database;
}  // namespace silo

namespace silo::query_engine::actions {

Fasta::Fasta() = default;

QueryResult Fasta::execute(const Database& database, std::vector<OperatorResult> bitmap_filter)
   const {
   return {};
}

void from_json(const nlohmann::json& /*json*/, std::unique_ptr<Fasta>& action) {
   action = std::make_unique<Fasta>();
   throw QueryParseException("Not implemented: The Fasta action has not been implemented");
}

}  // namespace silo::query_engine::actions
