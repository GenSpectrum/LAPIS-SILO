#pragma once

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {

struct GroupByField {
   std::string name;
};

class Aggregated : public Action {
  public:
   std::vector<GroupByField> group_by_fields;

   explicit Aggregated(std::vector<std::string> group_by_fields);
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Aggregated>& action);

}  // namespace silo::query_engine::actions
