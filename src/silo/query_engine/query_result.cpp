#include "silo/query_engine/query_result.h"

#include <nlohmann/json.hpp>

#include "silo_api/variant_json_serializer.h"

namespace silo::query_engine {
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MutationProportion, mutation, proportion, count)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ErrorResult, error, message)

// NOLINTNEXTLINE(readability-identifier-naming)
void to_json(nlohmann::json& json, const QueryResult& query_result) {
   json = nlohmann::json{
      {"queryResult", query_result.query_result},
   };
}

// NOLINTNEXTLINE(readability-identifier-naming)
void to_json(nlohmann::json& json, const AggregationResult& aggregation_result) {
   for (auto& [field, value] : aggregation_result.fields) {
      json[field] = value;
   }
}

}  // namespace silo::query_engine
