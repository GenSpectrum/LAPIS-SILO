#pragma once

#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/actions/action.h"

#include <arrow/acero/exec_plan.h>
#include <nlohmann/json.hpp>

namespace silo::query_engine::actions {

class Details : public Action {
  public:
   std::vector<std::string> fields;

   explicit Details(std::vector<std::string> fields);
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Details>& action);

}  // namespace silo::query_engine::actions
