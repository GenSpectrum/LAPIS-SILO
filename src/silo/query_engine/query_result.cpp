#include "silo/query_engine/query_result.h"

#include <nlohmann/json.hpp>

#include "silo_api/variant_json_serializer.h"

namespace silo::query_engine {

// NOLINTNEXTLINE(readability-identifier-naming)
void to_json(nlohmann::json& json, const QueryResultEntry& result_entry) {
   for (const auto& [field, value] : result_entry.fields) {
      if (value.has_value()) {
         json[field] = value.value();
      } else {
         json[field] = nlohmann::json();
      }
   }
}

}  // namespace silo::query_engine
