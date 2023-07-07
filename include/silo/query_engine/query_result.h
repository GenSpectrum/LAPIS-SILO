#ifndef SILO_QUERY_ENGINE_RESULT_H
#define SILO_QUERY_ENGINE_RESULT_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace silo::query_engine {

struct QueryResultEntry {
   std::map<std::string, std::optional<std::variant<std::string, int32_t, double>>> fields;
};

struct QueryResult {
   std::vector<QueryResultEntry> query_result;
};

// NOLINTBEGIN(readability-identifier-naming)
void to_json(nlohmann::json& json, const QueryResultEntry& result_entry);
void to_json(nlohmann::json& json, const QueryResult& query_result);
// NOLINTEND(readability-identifier-naming)

}  // namespace silo::query_engine

#endif  // SILO_QUERY_ENGINE_RESULT_H
