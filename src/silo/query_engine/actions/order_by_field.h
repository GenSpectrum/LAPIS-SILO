#pragma once

#include <string>

#include <nlohmann/json_fwd.hpp>

namespace silo::query_engine {

struct OrderByField {
   std::string name;
   bool ascending;
};

}  // namespace silo::query_engine
