#include "silo/query_engine/actions/details.h"

#include <nlohmann/json.hpp>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_result.h"

namespace silo::query_engine::actions {

Details::Details() = default;

QueryResult Details::execute(
   const silo::Database& database,
   std::vector<silo::query_engine::OperatorResult> bitmap_filter
) const {
   return {ErrorResult{"Not implemented", "The Details action has not been implemented"}};
}

void from_json(const nlohmann::json& /*json*/, std::unique_ptr<Details>& action) {
   action = std::make_unique<Details>();
}

}  // namespace silo::query_engine::actions
