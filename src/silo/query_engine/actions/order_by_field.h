#pragma once

#include <string>

#include <nlohmann/json_fwd.hpp>

namespace silo::query_engine {

struct OrderByField {
   std::string name;
   bool ascending;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, OrderByField& field);

}  // namespace silo::query_engine
