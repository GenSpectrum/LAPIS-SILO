#include "silo/query_engine/query_result.h"

#include <nlohmann/json.hpp>

#include "silo_api/variant_json_serializer.h"

namespace silo::query_engine {

// NOLINTNEXTLINE(readability-identifier-naming)
void to_json(nlohmann::json& json, const QueryResult& query_result) {
   json = nlohmann::json{
      {"queryResult", query_result.query_result},
   };
}

// NOLINTNEXTLINE(readability-identifier-naming)
void to_json(nlohmann::json& json, const QueryResultEntry& result_entry) {
   for (const auto& [field, value] : result_entry.fields) {
      json[field] = value;
   }
}

}  // namespace silo::query_engine
