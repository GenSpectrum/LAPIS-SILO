#pragma once

#include <memory>
#include <string>
#include <vector>

#include <arrow/result.h>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/actions/action.h"

namespace silo::query_engine::actions {

class MostRecentCommonAncestor : public Action {
  public:
   std::string column_name;
   bool print_nodes_not_in_tree;

   MostRecentCommonAncestor(std::string column_name, bool print_nodes_not_in_tree);
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<MostRecentCommonAncestor>& action);

}  // namespace silo::query_engine::actions