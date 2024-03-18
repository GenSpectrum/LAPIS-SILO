#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/common/json_value_type.h"

namespace silo::query_engine {

struct QueryResultEntry {
   std::map<std::string, common::JsonValueType> fields;
};

struct QueryResult {
   std::vector<QueryResultEntry> query_result;
};

// NOLINTBEGIN(readability-identifier-naming)
void to_json(nlohmann::json& json, const QueryResultEntry& result_entry);
void to_json(nlohmann::json& json, const QueryResult& query_result);
// NOLINTEND(readability-identifier-naming)

}  // namespace silo::query_engine
