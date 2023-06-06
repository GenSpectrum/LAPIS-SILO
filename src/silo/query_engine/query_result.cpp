#include "silo/query_engine/query_result.h"

#include <nlohmann/json.hpp>

#include "silo_api/variant_json_serializer.h"

namespace silo::response {
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AggregationResult, count)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MutationProportion, mutation, proportion, count)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ErrorResult, error, message)

// NOLINTNEXTLINE(readability-identifier-naming)
void to_json(nlohmann::json& json, const QueryResult& query_result) {
   json = nlohmann::json{
      {"queryResult", query_result.query_result},
      {"parseTime", query_result.parse_time},
      {"filterTime", query_result.filter_time},
      {"actionTime", query_result.action_time},
   };
}

}  // namespace silo::response
